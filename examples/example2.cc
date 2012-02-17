/**
 *
 *  \file example2.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
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
  vita::environment env;

  // Size of the population.
  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  // Size of an individual.
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make("NUMBER", vita::d_double, -200, 200));
  env.insert(factory.make("ADD", vita::d_double));
  env.insert(factory.make("SUB", vita::d_double));
  env.insert(factory.make("MUL", vita::d_double));
  env.insert(factory.make("IFL", vita::d_double));
  env.insert(factory.make("IFE", vita::d_double));

  vita::population p(env);

  std::cout << p << std::endl;

  return EXIT_SUCCESS;
}
