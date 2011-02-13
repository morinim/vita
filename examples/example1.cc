/**
 *
 *  \file example1.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/30
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>

#include "vita.h"
#include "environment.h"
#include "individual.h"
#include "primitive/sr_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 10;

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  vita::individual i(env,true);

  i.dump(std::cout);
  std::cout << std::endl;
  i.list(std::cout);
  std::cout << std::endl;
  i.tree(std::cout);

  return EXIT_SUCCESS;
}
