/**
 *
 *  \file example1.cc
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

int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 10;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make(vita::d_double, -200, 200));
  env.insert(factory.make("FADD"));
  env.insert(factory.make("FSUB"));
  env.insert(factory.make("FMUL"));
  env.insert(factory.make("FIFL"));
  env.insert(factory.make("FIFE"));

  vita::individual i(env, true);

  i.dump(std::cout);
  std::cout << std::endl;
  i.list(std::cout);
  std::cout << std::endl;
  i.tree(std::cout);

  return EXIT_SUCCESS;
}
