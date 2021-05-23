// File: from_file_modulation.hpp
// Project: include
// Created Date: 17/05/2021
// Author: Shun Suzuki
// -----
// Last Modified: 23/05/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/modulation.hpp"

namespace autd::modulation {
/**
 * @brief Modulation created from raw pcm data
 */
class RawPCM final : public core::Modulation {
 public:
  /**
   * @brief Generate function
   * @param[in] filename file path to raw pcm data
   * @param[in] sampling_freq sampling frequency of the data
   * @details The sampling frequency of AUTD is shown in autd::MOD_SAMPLING_FREQ, and it is not possible to modulate beyond the Nyquist frequency.
   * No modulation beyond the Nyquist frequency can be produced.
   * If samplingFreq is less than the Nyquist frequency, the data will be upsampled.
   * The maximum modulation buffer size is shown in autd::MOD_BUF_SIZE. Only the data up to MOD_BUF_SIZE/MOD_SAMPLING_FREQ seconds can be output.
   * @return return Ok(ModulationPtr) if succeeded, or Err(error msg) if failed to read the file
   */
  static Result<core::ModulationPtr, std::string> create(const std::string& filename, double sampling_freq = 0.0);
  Error build(core::Configuration config) override;
  explicit RawPCM(const double sampling_freq, std::vector<uint8_t> buf) : Modulation(), _sampling_freq(sampling_freq), _buf(std::move(buf)) {}

 private:
  double _sampling_freq = 0;
  std::vector<uint8_t> _buf;
};

/**
 * @brief Modulation created from wav file
 */
class Wav final : public core::Modulation {
 public:
  /**
   * @brief Generate function
   * @param[in] filename file path to wav data
   * @details The sampling frequency of AUTD is shown in autd::MOD_SAMPLING_FREQ, and it is not possible to modulate beyond the Nyquist frequency.
   * No modulation beyond the Nyquist frequency can be produced.
   * If samplingFreq is less than the Nyquist frequency, the data will be upsampled.
   * The maximum modulation buffer size is shown in autd::MOD_BUF_SIZE. Only the data up to MOD_BUF_SIZE/MOD_SAMPLING_FREQ seconds can be output.
   * @return return Ok(ModulationPtr) if succeeded, or Err(error msg) if failed to read the file
   */
  static Result<core::ModulationPtr, std::string> create(const std::string& filename);
  Error build(core::Configuration config) override;
  explicit Wav(const uint32_t sampling_freq, std::vector<uint8_t> buf) : Modulation(), _sampling_freq(sampling_freq), _buf(std::move(buf)) {}

 private:
  uint32_t _sampling_freq = 0;
  std::vector<uint8_t> _buf;
};
}  // namespace autd::modulation
