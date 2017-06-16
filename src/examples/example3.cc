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

//
// Performs three types of crossover between two random individuals.
//
int main(int argc, char *argv[])
{
  vita::symbol_set sset;
  vita::environment env(&sset, true);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 10);

  vita::symbol_factory factory;;
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFE"));

  vita::i_mep i1(env), i2(env);

  std::cout << "PARENTS\n" << std::string(40, '-') << '\n'
            << vita::out::dump
            << i1 << '\n'
            << i2 << '\n'
            << "OFFSPRING\n" << std::string(40, '-') << '\n'
            << crossover(i1, i2)
            << std::endl;
}
