/**
 *
 *  \file example2.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/26
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>
#include <memory>

#include "vita.h"
#include "environment.h"
#include "evolution.h"
#include "primitive/sr_pri.h"

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

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  vita::population p(env);

  std::cout << p << std::endl;
      
  return EXIT_SUCCESS;
}
