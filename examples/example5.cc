/**
 *
 *  \file example5.cc
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
#include "kernel/interpreter.h"
#include "kernel/primitive/double_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 14;

  vita::symbol_ptr s1(new vita::sr::number(-200, 200));
  vita::symbol_ptr s2(new vita::sr::add());
  vita::symbol_ptr s3(new vita::sr::ife());
  vita::symbol_ptr s4(new vita::sr::ifl());
  vita::symbol_ptr s5(new vita::sr::ifz());
  vita::symbol_ptr s6(new vita::sr::mod());
  vita::symbol_ptr s7(new vita::sr::mul());
  vita::symbol_ptr s8(new vita::sr::sub());
  env.insert(s1);
  env.insert(s2);
  env.insert(s3);
  env.insert(s4);
  env.insert(s5);
  env.insert(s6);
  env.insert(s7);
  env.insert(s8);

  vita::individual ind(env, true);

  ind.list(std::cout);
  std::cout << std::endl;

  vita::interpreter agent(ind);
  const boost::any val(agent());
  if (val.empty())
    std::cout << "Incorrect program." << std::endl;
  else
    std::cout << "Output: " << boost::any_cast<double>(val) << std::endl;

  return EXIT_SUCCESS;
}
