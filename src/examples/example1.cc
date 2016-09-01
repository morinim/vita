/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
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
  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 10);

  vita::symbol_factory factory;
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFE"));

  vita::i_mep i(env);

  i.dump(std::cout);
  std::cout << '\n' << i << '\n';
  i.tree(std::cout);
}
