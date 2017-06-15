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
  vita::symbol_set sset;
  vita::environment env(&sset, true);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 14);

  vita::symbol_factory factory;
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
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

  vita::i_mep ind(env);

  dump(ind);
  std::cout << '\n';

  const vita::any val(vita::interpreter<vita::i_mep>(&ind).run());
  if (val.empty())
    std::cout << "Incorrect program.\n";
  else
    std::cout << "Output: " << vita::to<std::string>(val) << '\n';
}
