/**
 *
 *  \file example5.cc
 *
 *  \author Manlio Morini
 *  \date 2010/02/10
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>

#include "environment.h"
#include "interpreter.h"
#include "individual.h"
#include "primitive/sr_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 14;

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::ife());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ifz());
  env.insert(new vita::sr::mod());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::sub());

  vita::individual ind(env,true);

  ind.list(std::cout);  
  std::cout << std::endl;

  const boost::any val(vita::interpreter(ind).run());
  if (val.empty())
    std::cout << "Incorrect program." << std::endl;
  else
    std::cout << "Output: " << boost::any_cast<double>(val) << std::endl;

  return EXIT_SUCCESS;
}
