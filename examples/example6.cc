/**
 *
 *  \file example6.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>

#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/interpreter.h"
#include "kernel/terminal.h"
#include "kernel/primitive/factory.h"

// This class models the first input.
class X : public vita::terminal
{
public:
  X() : vita::terminal("X", 0, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

class Y : public vita::terminal
{
public:
  Y() : vita::terminal("Y", 0, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

class Z : public vita::terminal
{
public:
  Z() : vita::terminal("Z", 0, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

double X::val;
double Y::val;
double Z::val;

class fitness : public vita::evaluator
{
  vita::score_t operator()(const vita::individual &ind)
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
            assert(std::isfinite(dres));
            fit += std::exp(-std::fabs(dres - (x*x + y*y - z*z)));
          }
        }

    return vita::score_t(fit, -1.0);
  }
};

int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;
  env.g_since_start = argc > 3 ? atoi(argv[3]) : 100;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(std::make_shared<X>());
  env.insert(std::make_shared<Y>());
  env.insert(std::make_shared<Z>());
  env.insert(factory.make("FADD"));
  env.insert(factory.make("FSUB"));
  env.insert(factory.make("FMUL"));
  env.insert(factory.make("FIFL"));
  env.insert(factory.make("FIFE"));

  std::unique_ptr<vita::evaluator> eva(new fitness());

  vita::evolution(env, eva.get())(true, 1);

  return EXIT_SUCCESS;
}
