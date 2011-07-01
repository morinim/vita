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
#include "kernel/primitive/sr_pri.h"

//
// Performs three types of crossover between two random individuals.
//
int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 10;

  env.insert(new vita::sr::number(-200, 200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  vita::individual i1(env, true), i2(env, true);

  std::cout << "PARENTS" << std::endl << std::string(40, '-') << std::endl;
  i1.dump(std::cout);
  std::cout << std::endl;
  i2.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (UNIFORM CROSSOVER)" << std::endl
            << std::string(40, '-') << std::endl;

  const vita::individual uc(i1.uniform_cross(i2));
  uc.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (ONE POINT CROSSOVER)" << std::endl
            << std::string(40, '-') << std::endl;

  const vita::individual upc(i1.cross1(i2));
  upc.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (TWO POINTS CROSSOVER)" << std::endl
            << std::string(40, '-') << std::endl;

  const vita::individual tpc(i1.cross2(i2));
  tpc.dump(std::cout);

  return EXIT_SUCCESS;
}
