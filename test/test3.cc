/**
 *
 *  \file test3.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/19
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>

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
  
  const vita::individual orig(i);

  env.p_mutation = 0;
  i.mutation();
  if (i != orig)
    std::cerr << "Mutation error." << std::endl;

  i.dump(std::cout);
  std::cout << std::endl;

  env.p_mutation = 1;
  i.mutation();

  i.dump(std::cout);
  std::cout << std::endl;

  env.p_mutation = 0.5;
  double dist(0.0);
  const unsigned n(100);
  for (unsigned j(0); j < n; ++j)
  { 
    const vita::individual i1(i);

    i.mutation();
    dist += i1.distance(i);
  }

  i.dump(std::cout);
  std::cout << std::endl;

  dist /= n;
  std::cout << 100*dist/env.code_length << '%' << std::endl;
  
  return EXIT_SUCCESS;
}
