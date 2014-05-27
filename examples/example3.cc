/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/environment.h"
#include "kernel/individual/mep.h"
#include "kernel/src/primitive/factory.h"

//
// Performs three types of crossover between two random individuals.
//
int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 10);

  vita::symbol_set sset;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFE"));

  vita::individual i1(env, sset), i2(env, sset);

  std::cout << "PARENTS" << std::endl << std::string(40, '-') << std::endl;
  i1.dump(std::cout);
  std::cout << std::endl;
  i2.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING" << std::endl
            << std::string(40, '-') << std::endl;

  i1.crossover(i2).dump(std::cout);
  std::cout << std::endl;

  return EXIT_SUCCESS;
}
