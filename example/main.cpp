// File: main.cpp
// Project: example
// Created Date: 17/05/2021
// Author: Shun Suzuki
// -----
// Last Modified: 03/06/2021
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
  auto adapters = autd::link::SOEMLink::enumerate_adapters();
  for (size_t i = 0; i < adapters.size(); i++) {
    auto& [fst, snd] = adapters[i];
    cout << "[" << i << "]: " << fst << ", " << snd << endl;
  }

  int index;
  cout << "Choose number: ";
  cin >> index;
  cin.ignore();

  return adapters[index].name;
}

int main() {
  try {
    auto autd = autd::Controller::create();
    autd->geometry()->add_device(autd::Vector3(0, 0, 0), autd::Vector3(0, 0, 0));
    const auto ifname = GetAdapterName();
    if (auto res = autd->open(autd::link::SOEMLink::create(ifname, autd->geometry()->num_devices())); res.is_err()) {
      std::cerr << res.unwrap_err() << std::endl;
      return ENXIO;
    }

    autd->geometry()->wavelength() = 8.5;

    autd->clear().unwrap();
    autd->synchronize().unwrap();

    autd->silent_mode() = true;

    const autd::Vector3 center(TRANS_SPACING_MM * ((NUM_TRANS_X - 1) / 2.0), TRANS_SPACING_MM * ((NUM_TRANS_Y - 1) / 2.0), 150.0);
    const auto g = autd::gain::FocalPoint::create(center);

    cout << "RawPCM test" << endl;
    const auto raw_pcm = autd::modulation::RawPCM::create("sin150.dat", 4000).unwrap();
    autd->send(g, raw_pcm).unwrap();

    cout << "press any key to finish..." << endl;
    cin.ignore();
    autd->stop().unwrap();

    cout << "press any key to start Wave test..." << endl;
    cin.ignore();

    cout << "Wave test" << endl;
    const auto wav = autd::modulation::Wav::create("sin150.wav").unwrap();
    autd->send(g, wav).unwrap();

    cout << "press any key to finish..." << endl;
    cin.ignore();

    autd->clear().unwrap();
    autd->close().unwrap();

  } catch (exception& e) {
    std::cerr << e.what() << std::endl;
    return ENXIO;
  }
  return 0;
}
