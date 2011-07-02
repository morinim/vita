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
#include "kernel/primitive/sr_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 14;

  env.insert(new vita::sr::number(-200, 200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::ife());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ifz());
  env.insert(new vita::sr::mod());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::sub());

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
