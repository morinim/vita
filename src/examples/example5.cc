/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <iostream>

#include "kernel/vita.h"

int main(int argc, char *argv[])
{
  using namespace vita;
  symbol_set sset;
  environment env(&sset, initialization::standard);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 14);

  symbol_factory factory;
  sset.insert(factory.make(domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FIFE"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFZ"));
  sset.insert(factory.make("FMOD"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FLENGTH", {1, 0}));
  sset.insert(factory.make("apple", {1}));
  sset.insert(factory.make("grapefruit", {1}));
  sset.insert(factory.make("orange", {1}));

  i_mep ind(env);

  std::cout << out::dump << ind << '\n';

  const any val(interpreter<i_mep>(&ind).run());
  if (val.has_value())
    std::cout << "Output: " << to<std::string>(val) << '\n';
  else
    std::cout << "Incorrect program.\n";
}
