// File: main.cpp
// Project: example
// Created Date: 17/05/2021
// Author: Shun Suzuki
// -----
// Last Modified: 17/05/2021
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2021 Hapis Lab. All rights reserved.
//

#include <iostream>

#include "autd3.hpp"
#include "from_file_modulation.hpp"
#include "primitive_gain.hpp"
#include "soem_link.hpp"

using autd::NUM_TRANS_X, autd::NUM_TRANS_Y, autd::TRANS_SPACING_MM;
using namespace std;

string GetAdapterName() {
  size_t size;
  auto adapters = autd::link::SOEMLink::EnumerateAdapters(&size);
  for (size_t i = 0; i < size; i++) {
    auto& [fst, snd] = adapters[i];
    cout << "[" << i << "]: " << fst << ", " << snd << endl;
  }

  int index;
  cout << "Choose number: ";
  cin >> index;
  cin.ignore();

  return adapters[index].second;
}

int main() {
  try {
    autd::Controller autd;
    autd.geometry()->AddDevice(autd::Vector3(0, 0, 0), autd::Vector3(0, 0, 0));
    const auto ifname = GetAdapterName();
    if (auto res = autd.OpenWith(autd::link::SOEMLink::Create(ifname, autd.geometry()->num_devices())); res.is_err()) {
      std::cerr << res.unwrap_err() << std::endl;
      return ENXIO;
    }

    autd.geometry()->wavelength() = 8.5;

    autd.Clear().unwrap();
    autd.Synchronize().unwrap();

    autd.silent_mode() = true;

    const autd::Vector3 center(TRANS_SPACING_MM * ((NUM_TRANS_X - 1) / 2.0), TRANS_SPACING_MM * ((NUM_TRANS_Y - 1) / 2.0), 150.0);
    const auto g = autd::gain::FocalPoint::Create(center);

    cout << "RawPCM test" << endl;
    const auto raw_pcm = autd::modulation::RawPCM::Create("sin150.dat", 4000).unwrap();
    autd.Send(g, raw_pcm).unwrap();

    cout << "press any key to finish..." << endl;
    cin.ignore();

    cout << "Wave test" << endl;
    const auto wav = autd::modulation::Wav::Create("sin150.wav").unwrap();
    autd.Send(g, wav).unwrap();

    cout << "press any key to finish..." << endl;
    cin.ignore();

    autd.Clear().unwrap();
    autd.Close().unwrap();

  } catch (exception& e) {
    std::cerr << e.what() << std::endl;
    return ENXIO;
  }
  return 0;
}
