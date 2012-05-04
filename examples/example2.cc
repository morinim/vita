/**
 *
 *  \file example2.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>
#include <memory>

#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/primitive/factory.h"

//
// Creates a random population and prints it out.
//
int main(int argc, char *const argv[])
{
  vita::environment env(true);

  // Size of the population.
  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  // Size of an individual.
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make(vita::d_double, -200, 200));
  env.insert(factory.make("FADD"));
  env.insert(factory.make("FSUB"));
  env.insert(factory.make("FMUL"));
  env.insert(factory.make("FIFL"));
  env.insert(factory.make("FIFE"));

  vita::population p(env);

  std::cout << p << std::endl;

  return EXIT_SUCCESS;
}
