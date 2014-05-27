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
#include <memory>

#include "kernel/environment.h"
#include "kernel/individual/mep.h"
#include "kernel/population.h"
#include "kernel/src/primitive/factory.h"

//
// Creates a random population and prints it out.
//
int main(int argc, char *const argv[])
{
  vita::environment env(true);

  // Size of the population.
  env.individuals = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 100);
  // Size of an individual.
  env.code_length = static_cast<unsigned>(argc > 2 ? std::atoi(argv[2]) : 100);

  vita::symbol_set sset;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFE"));

  vita::population<vita::individual> p(env, sset);

  std::cout << p << std::endl;

  return EXIT_SUCCESS;
}
