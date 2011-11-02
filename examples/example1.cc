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
#include "kernel/primitive/double_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 10;

  vita::symbol_ptr s1(new vita::sr::number(-200, 200));
  vita::symbol_ptr s2(new vita::sr::add());
  vita::symbol_ptr s3(new vita::sr::sub());
  vita::symbol_ptr s4(new vita::sr::mul());
  vita::symbol_ptr s5(new vita::sr::ifl());
  vita::symbol_ptr s6(new vita::sr::ife());
  env.insert(s1);
  env.insert(s2);
  env.insert(s3);
  env.insert(s4);
  env.insert(s5);
  env.insert(s6);

  vita::individual i(env, true);

  i.dump(std::cout);
  std::cout << std::endl;
  i.list(std::cout);
  std::cout << std::endl;
  i.tree(std::cout);

  return EXIT_SUCCESS;
}
