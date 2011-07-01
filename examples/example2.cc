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

#include "kernel/vita.h"
#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/primitive/sr_pri.h"

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

  env.insert(new vita::sr::number(-200, 200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  vita::population p(&env);

  std::cout << p << std::endl;

  return EXIT_SUCCESS;
}
