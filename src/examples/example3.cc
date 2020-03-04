/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \example example3.cc
 *  Performs various types of crossover between random individuals.
 */

#include <iostream>

#include "kernel/vita.h"

//
// Performs three types of crossover between two random individuals.
//
int main(int argc, char *argv[])
{
  using namespace vita;
  problem p;

  p.env.init();
  p.env.mep.code_length = static_cast<unsigned>(argc > 1
                                                ? std::atoi(argv[1])
                                                : 10);

  symbol_factory factory;;
  p.sset.insert(factory.make(domain_t::d_double, -200, 200));
  p.sset.insert(factory.make("FADD"));
  p.sset.insert(factory.make("FSUB"));
  p.sset.insert(factory.make("FMUL"));
  p.sset.insert(factory.make("FIFL"));
  p.sset.insert(factory.make("FIFE"));

  i_mep i1(p), i2(p);

  std::cout << "PARENTS\n" << std::string(40, '-') << '\n'
            << out::dump
            << i1 << '\n'
            << i2 << '\n'
            << "OFFSPRING\n" << std::string(40, '-') << '\n'
            << crossover(i1, i2)
            << std::endl;
}
