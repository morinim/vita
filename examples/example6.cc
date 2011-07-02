/**
 *
 *  \file example6.cc
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
#include <fstream>
#include <memory>

#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/primitive/sr_pri.h"

// This class models the first input.
class X : public vita::terminal
{
public:
  X() : vita::terminal("X", vita::sym_real, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

class Y : public vita::terminal
{
public:
  Y() : vita::terminal("Y", vita::sym_real, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

class Z : public vita::terminal
{
public:
  Z() : vita::terminal("Z", vita::sym_real, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

double X::val;
double Y::val;
double Z::val;

class fitness : public vita::evaluator
{
  vita::fitness_t operator()(const vita::individual &ind)
  {
    vita::interpreter agent(ind);

    vita::fitness_t fit(0.0);
    for (double x(0); x < 10; ++x)
      for (double y(0); y < 10; ++y)
        for (double z(0); z < 10; ++z)
        {
          X::val = x;
          Y::val = y;
          Z::val = z;

          const boost::any res(agent());

          if (!res.empty())
          {
            const double dres(boost::any_cast<double>(res));
            assert(!vita::is_bad(dres));
            fit += std::exp(-std::fabs(dres - (x*x+y*y-z*z)));
          }
        }

    return fit;
  }
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

  vita::evolution(&env, eva.get())(true, 1);

  env.sset.delete_symbols();

  return EXIT_SUCCESS;
}
