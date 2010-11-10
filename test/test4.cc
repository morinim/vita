/**
 *
 *  \file test4.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/20
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

  vita::individual i1(env,true), i2(env,true);

  std::cout << "PARENTS" << std::endl << std::string(40,'-') << std::endl; 
  i1.dump(std::cout);
  std::cout << std::endl;
  i2.dump(std::cout);
  std::cout << std::endl;
  
  std::cout << "OFFSPRING (UNIFORM CROSSOVER)" << std::endl 
	    << std::string(40,'-') << std::endl;

  const vita::individual uc(i1.uniform_cross(i2));
  uc.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (HOMOLOGOUS CROSSOVER)" << std::endl 
	    << std::string(40,'-') << std::endl;

  //const vita::individual hc(i1.hcross(i2));
  //hc.dump(std::cout);
  //std::cout << std::endl;

  std::cout << "OFFSPRING (ONE POINT CROSSOVER)" << std::endl 
	    << std::string(40,'-') << std::endl;

  const vita::individual upc(i1.cross1(i2));
  upc.dump(std::cout);
  std::cout << std::endl;

  std::cout << "OFFSPRING (TWO POINTS CROSSOVER)" << std::endl 
	    << std::string(40,'-') << std::endl;

  const vita::individual tpc(i1.cross2(i2));
  tpc.dump(std::cout);
  std::cout << std::endl;

  const unsigned n(argc > 2 ? atoi(argv[2]) : 100);
  std::cout << "AVERAGE DISTANCES" << std::endl << std::string(40,'-') 
	    << std::endl; 
  double dist1(0.0), dist2(0.0), dist3(0.0), dist4(0.0);
  for (unsigned j(0); j < n; ++j)
  { 
    const vita::individual tmp1(i1.uniform_cross(i2));
    dist1 += i1.distance(tmp1);
    //const vita::individual tmp2(i1.hcross(i2));
    //dist2 += i1.distance(tmp2);
    const vita::individual tmp3(i1.cross1(i2));
    dist3 += i1.distance(tmp3);
    const vita::individual tmp4(i1.cross2(i2));
    dist4 += i1.distance(tmp4);
  }

  dist1 /= n;
  dist2 /= n;
  dist3 /= n;
  dist4 /= n;
  std::cout << "Uniform crossover: " << 100*dist1/env.code_length << '%' 
	    << std::endl
	    << "Homologous crossover: " << 100*dist2/env.code_length << '%'
	    << std::endl
	    << "One point crossover: " << 100*dist3/env.code_length << '%'
	    << std::endl
	    << "Two points crossover: " << 100*dist4/env.code_length << '%'
	    << std::endl;

  return EXIT_SUCCESS;
}
