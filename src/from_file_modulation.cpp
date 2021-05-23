// File: from_file_modulation.cpp
// Project: src
// Created Date: 17/05/2021
// Author: Shun Suzuki
// -----
// Last Modified: 23/05/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#include "from_file_modulation.hpp"

#include <cmath>
#include <cstring>
#include <fstream>

namespace autd::modulation {
namespace {
double Sinc(const double x) noexcept {
  if (fabs(x) < std::numeric_limits<double>::epsilon()) return 1;
  return std::sin(M_PI * x) / (M_PI * x);
}
}  // namespace

Result<core::ModulationPtr, std::string> RawPCM::Create(const std::string& filename, const double sampling_freq) {
  std::ifstream ifs;
  ifs.open(filename, std::ios::binary);

  if (ifs.fail()) return Err(std::string("Error on opening file"));

  std::vector<uint8_t> tmp;
  char buf[sizeof(uint8_t)];
  while (ifs.read(buf, sizeof(uint8_t))) {
    int value;
    std::memcpy(&value, buf, sizeof(uint8_t));
    tmp.emplace_back(value);
  }

  core::ModulationPtr mod = std::make_shared<RawPCM>(sampling_freq, tmp);
  return Ok(std::move(mod));
}

Error RawPCM::Build(const core::Configuration config) {
  const auto mod_sf = static_cast<int32_t>(config.mod_sampling_freq());
  if (this->_sampling_freq < std::numeric_limits<double>::epsilon()) this->_sampling_freq = static_cast<double>(mod_sf);

  // up sampling
  std::vector<int32_t> sample_buf;
  const auto freq_ratio = static_cast<double>(mod_sf) / _sampling_freq;
  sample_buf.resize(this->_buf.size() * static_cast<size_t>(freq_ratio));
  for (size_t i = 0; i < sample_buf.size(); i++) {
    const auto v = static_cast<double>(i) / freq_ratio;
    const auto tmp = std::fmod(v, double{1}) < 1 / freq_ratio ? this->_buf.at(static_cast<int>(v)) : 0;
    sample_buf.at(i) = tmp;
  }

  // LPF
  const auto num_tap = 31;
  const auto cutoff = _sampling_freq / 2 / static_cast<double>(mod_sf);
  std::vector<double> lpf(num_tap);
  for (auto i = 0; i < num_tap; i++) {
    const auto t = i - num_tap / 2;
    lpf.at(i) = Sinc(static_cast<double>(t) * cutoff * 2);
  }

  auto max_v = std::numeric_limits<double>::min();
  auto min_v = std::numeric_limits<double>::max();
  std::vector<double> lpf_buf;
  lpf_buf.resize(sample_buf.size(), 0);
  for (size_t i = 0; i < lpf_buf.size(); i++) {
    for (auto j = 0; j < num_tap; j++) {
      lpf_buf.at(i) += static_cast<double>(sample_buf.at((i - j + sample_buf.size()) % sample_buf.size())) * lpf.at(j);
    }
    max_v = std::max<double>(lpf_buf.at(i), max_v);
    min_v = std::min<double>(lpf_buf.at(i), min_v);
  }

  if (max_v - min_v < std::numeric_limits<double>::epsilon()) max_v = min_v + 1;
  this->buffer().resize(lpf_buf.size(), 0);
  for (size_t i = 0; i < lpf_buf.size(); i++) {
    this->buffer().at(i) = static_cast<uint8_t>(round(255 * ((lpf_buf.at(i) - min_v) / (max_v - min_v))));
  }
  return Ok();
}

namespace {
template <class T>
Result<T, std::string> ReadFromStream(std::ifstream& fsp) {
  char buf[sizeof(T)];
  if (!fsp.read(buf, sizeof(T))) return Err(std::string("Invalid data length."));
  T v{};
  std::memcpy(&v, buf, sizeof(T));
  return Ok(v);
}
}  // namespace

Result<core::ModulationPtr, std::string> Wav::Create(const std::string& filename) {
  std::ifstream fs;
  fs.open(filename, std::ios::binary);
  if (fs.fail()) return Err(std::string("Error on opening file."));

  if (const auto riff_tag = ReadFromStream<uint32_t>(fs).unwrap_or(0); riff_tag != 0x46464952u) return Err(std::string("Invalid data format."));

  [[maybe_unused]] const auto chunk_size = ReadFromStream<uint32_t>(fs);

  if (const auto wav_desc = ReadFromStream<uint32_t>(fs).unwrap_or(0); wav_desc != 0x45564157u) return Err(std::string("Invalid data format."));

  if (const auto fmt_desc = ReadFromStream<uint32_t>(fs).unwrap_or(0); fmt_desc != 0x20746d66u) return Err(std::string("Invalid data format."));

  if (const auto fmt_chunk_size = ReadFromStream<uint32_t>(fs).unwrap_or(0); fmt_chunk_size != 0x00000010u)
    return Err(std::string("Invalid data format."));

  if (const auto wave_fmt = ReadFromStream<uint16_t>(fs).unwrap_or(0); wave_fmt != 0x0001u)
    return Err(std::string("Invalid data format. This supports only uncompressed linear PCM data."));

  if (const auto channel = ReadFromStream<uint16_t>(fs).unwrap_or(0); channel != 0x0001u)
    return Err(std::string("Invalid data format. This supports only monaural audio."));

  const auto sample_freq = ReadFromStream<uint32_t>(fs).unwrap_or(0);
  [[maybe_unused]] const auto bytes_per_sec = ReadFromStream<uint32_t>(fs).unwrap_or(0);
  [[maybe_unused]] const auto block_size = ReadFromStream<uint16_t>(fs).unwrap_or(0);
  const auto bits_per_sample = ReadFromStream<uint16_t>(fs).unwrap_or(0);

  if (const auto data_desc = ReadFromStream<uint32_t>(fs).unwrap_or(0); data_desc != 0x61746164u) return Err(std::string("Invalid data format."));

  const auto data_chunk_size = ReadFromStream<uint32_t>(fs).unwrap_or(0);

  if (bits_per_sample != 8 && bits_per_sample != 16) return Err(std::string("This only supports 8 or 16 bits per sampling data."));

  std::vector<uint8_t> tmp;
  const auto data_size = data_chunk_size / (bits_per_sample / 8);
  for (size_t i = 0; i < data_size; i++) {
    if (bits_per_sample == 8) {
      auto d = ReadFromStream<uint8_t>(fs).unwrap_or(0);
      tmp.emplace_back(d);
    } else if (bits_per_sample == 16) {
      const auto d32 = static_cast<int32_t>(ReadFromStream<int16_t>(fs).unwrap_or(0)) - std::numeric_limits<int16_t>::min();
      auto d8 = static_cast<uint8_t>(static_cast<double>(d32) / std::numeric_limits<uint16_t>::max() * std::numeric_limits<uint8_t>::max());
      tmp.emplace_back(d8);
    }
  }

  core::ModulationPtr mod = std::make_shared<Wav>(sample_freq, tmp);
  return Ok(std::move(mod));
}

Error Wav::Build(const core::Configuration config) {
  const auto mod_sf = static_cast<int32_t>(config.mod_sampling_freq());
  const auto mod_buf_size = static_cast<size_t>(config.mod_buf_size());

  // down sampling
  std::vector<uint8_t> sample_buf;
  const auto freq_ratio = mod_sf / static_cast<double>(_sampling_freq);
  auto buffer_size = static_cast<size_t>(static_cast<double>(this->_buf.size()) * freq_ratio);
  buffer_size = std::min(buffer_size, mod_buf_size);

  sample_buf.resize(buffer_size);
  for (size_t i = 0; i < sample_buf.size(); i++) {
    const auto idx = static_cast<size_t>(static_cast<double>(i) / freq_ratio);
    sample_buf.at(i) = _buf.at(idx);
  }

  this->buffer() = std::move(sample_buf);
  return Ok();
}
}  // namespace autd::modulation
