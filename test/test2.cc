/**
 *
 *  \file test2.cc
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>

#include "vita.h"
#include "environment.h"
#include "population.h"
#include "primitive/sr_pri.h"

int fitness(const vita::individual &)
{
  return 0;
}

int main(int argc, char *argv[])
{
  vita::environment env;

  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  vita::population p(env);

  std::cout << p << std::endl;

  vita::analyzer ay;
  p.pick_stats(&ay);

  const unsigned long long nef(ay.functions(true));
  const unsigned long long net(ay.terminals(true));
  const unsigned long long ne(nef+net);

  std::cout << std::string(40,'-') << std::endl;
  for (vita::analyzer::const_iterator i(ay.begin()); i != ay.end(); ++i)
    std::cout << std::setfill(' ') << (i->first)->display() << ": " 
	      << std::setw(5) << i->second.counter[true]
	      << " (" << std::setw(3) << 100*i->second.counter[true]/ne 
	      << "%)" << std::endl;

  std::cout << "Average code length: " << ay.length_dist().mean << std::endl;
  std::cout << "Code length standard deviation: " 
	    << std::sqrt(ay.length_dist().variance) << std::endl;
  std::cout << "Max code length: " << ay.length_dist().max << std::endl;
  std::cout << "Functions: " << nef << " (" << nef*100/ne << "%)" << std::endl;
  std::cout << "Terminals: " << net << " (" << net*100/ne << "%)" << std::endl;
  std::cout << std::string(40,'-') << std::endl;
      
  return EXIT_SUCCESS;
}
