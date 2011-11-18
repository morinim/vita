/**
 *
 *  \file example3.cc
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

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/primitive/factory.h"

//
// Performs three types of crossover between two random individuals.
//
int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 10;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make("NUMBER", vita::d_double, -200, 200));
  env.insert(factory.make("ADD", vita::d_double));
  env.insert(factory.make("SUB", vita::d_double));
  env.insert(factory.make("MUL", vita::d_double));
  env.insert(factory.make("IFL", vita::d_double));
  env.insert(factory.make("IFE", vita::d_double));

  vita::individual i1(env, true), i2(env, true);

  std::cout << "PARENTS" << std::endl << std::string(40, '-') << std::endl;
  i1.dump(std::cout);
  std::cout << std::endl;
  i2.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (UNIFORM CROSSOVER)" << std::endl
            << std::string(40, '-') << std::endl;

  uniform_crossover(i1, i2).dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (ONE POINT CROSSOVER)" << std::endl
            << std::string(40, '-') << std::endl;

  one_point_crossover(i1, i2).dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (TWO POINTS CROSSOVER)" << std::endl
            << std::string(40, '-') << std::endl;

  two_point_crossover(i1, i2).dump(std::cout);
  return EXIT_SUCCESS;
}
