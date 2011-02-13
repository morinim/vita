/**
 *
 *  \file example6.cc
 *
 *  \author Manlio Morini
 *  \date 2011/02/10
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

// This class models the first input.
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
    vita::interpreter agent(ind);

    double err(0);
    for (double x(0); x < 10; ++x)
      for (double y(0); y < 10; ++y)
	for (double z(0); z < 10; ++z)
	{
	  X::val = x;
	  Y::val = y;
	  Z::val = z;

	  const boost::any res(agent.run());

	  if (res.empty())
	    err += 1000;
	  else
	  {
	    const double dres(boost::any_cast<double>(res));
	    assert(!is_bad(dres));
	    err += std::fabs(dres - (x*x+y*y-z*z));
	  }
	}

    return -err;
  };
};

int main(int argc, char *argv[])
{
  vita::environment env;
  
  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;
  env.g_since_start = argc > 3 ? atoi(argv[3]) : 100;

  env.insert(new X());
  env.insert(new Y());
  env.insert(new Z());
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  std::auto_ptr<vita::evaluator> eva(new fitness());

  vita::evolution(env,eva.get()).run(true);

  return EXIT_SUCCESS;
}
