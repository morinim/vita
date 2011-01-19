/**
 *
 *  \file test7.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/09
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>

#include "environment.h"
#include "evolution.h"
#include "primitive/sr_pri.h"

class X : public vita::terminal
{
public:
  X() : vita::terminal("X",vita::sym_real) {};
  
  boost::any eval(vita::interpreter &) const { return val; };

  static double val;
};

class Y : public vita::terminal
{
public:
  Y() : vita::terminal("Y",vita::sym_real) {};
  
  boost::any eval(vita::interpreter &) const { return val; };

  static double val;
};

class Z : public vita::terminal
{
public:
  Z() : vita::terminal("Z",vita::sym_real) {};
  
  boost::any eval(vita::interpreter &) const { return val; };

  static double val;
};

double X::val;
double Y::val;
double Z::val;

class fitness : public vita::evaluator
{
  vita::fitness_t run(const vita::individual &ind)
  {
    //static double best_so_far(10000000);

    vita::interpreter agent(ind);

    double err(0);

    /*
      std::ofstream out("input.csv");
      for (unsigned r1(1); r1 <= 6; ++r1)
        for (unsigned r2(1); r2 <= 6; ++r2)
          for (unsigned r3(1); r3 <= 6; ++r3)
	    out << r1 << ' ' << r2 << ' ' << r3 << ' '
	        << (r1%2 ? r1-1 : 0) +
	           (r2%2 ? r2-1 : 0) +
                   (r3%2 ? r3-1 : 0)
		<< std::endl;
    */

    /*
      std::ofstream out("input.csv");
      for (unsigned r1(0); r1 <= 1; ++r1)
        for (unsigned r2(0); r2 <= 1; ++r2)
	  for (unsigned r3(0); r3 <= 1; ++r3)
	    for (unsigned r4(0); r4 <= 1; ++r4)
	      for (unsigned r5(0); r5 <= 1; ++r5)
	        out << r1 << ' ' << r2 << ' ' << r3 << ' ' << r4 << ' ' 
		    << r5 << ' ' << ((r1+r2+r3+r4)%2 ? 0 : 1)
		    << std::endl;
    */

    for (double x(0); x < 10; ++x)
      for (double y(0); y < 10; ++y)
	for (double z(0); z < 10; ++z)
	{
	  X::val = x;
	  Y::val = y;
	  Z::val = z;

	  const boost::any res(agent.run());

	  //if (res > 1000 || res < -1000)
	  //err += 1000;
	  if (res.empty())
	    err += 1000;
	  else
	  {
	    const double dres(boost::any_cast<double>(res));
	    assert(!isnan(dres) && !isinf(dres));
	    err += std::fabs(dres - (x*x+y*y-z*z));
	  }
	  
	  //if (err > best_so_far)
	  //  break;
	  
	  //if (res != x*x+y*y+z*z)
	  //  ++err;
	}

    //if (err < best_so_far)
    //  best_so_far = err;

    return -err;
  };
};

int main(int argc, char *argv[])
{
  vita::environment env;
  
  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;
  env.g_since_start = argc > 3 ? atoi(argv[3]) : 100;

  //env.insert(new vita::sr::number<double>(-200,200));
  env.insert(new X());
  env.insert(new Y());
  env.insert(new Z());
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  std::auto_ptr<vita::evaluator> eva(new fitness());
  vita::evolution evo(env,eva.get());

  evo.run(true);

  return EXIT_SUCCESS;
}
