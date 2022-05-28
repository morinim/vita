/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <iostream>
#include <fstream>

#include "kernel/vita.h"

// This class models the first input.
class X : public vita::terminal
{
public:
  X() : vita::terminal("X", 0) {}

  bool input() const override { return true; }

  vita::value_t eval(vita::symbol_params &) const override { return val; }

  static double val;
};

class Y : public vita::terminal
{
public:
  Y() : vita::terminal("Y", 0) {}

  bool input() const override { return true; }

  vita::value_t eval(vita::symbol_params &) const override { return val; }

  static double val;
};

class Z : public vita::terminal
{
public:
  Z() : vita::terminal("Z", 0) {}

  bool input() const override { return true; }

  vita::value_t eval(vita::symbol_params &) const override { return val; }

  static double val;
};

double X::val;
double Y::val;
double Z::val;

class my_evaluator : public vita::evaluator<vita::i_mep>
{
  vita::fitness_t operator()(const vita::i_mep &ind) override
  {
    vita::interpreter<vita::i_mep> agent(&ind);

    vita::fitness_t::value_type fit(0.0);
    for (double x(0); x < 10; ++x)
      for (double y(0); y < 10; ++y)
        for (double z(0); z < 10; ++z)
        {
          X::val = x;
          Y::val = y;
          Z::val = z;

          const auto res(agent.run());

          if (vita::has_value(res))
          {
            const auto dres(std::get<vita::D_DOUBLE>(res));
            assert(std::isfinite(dres));
            fit += std::exp(-std::fabs(dres - (x*x + y*y - z*z)));
          }
        }

    return {fit};
  }
};

int main(int argc, char *argv[])
{
  using namespace vita;
  problem p;

  p.env.init();
  p.env.individuals = static_cast<unsigned>(argc > 1
                                            ? std::atoi(argv[1])
                                            : 100);
  p.env.mep.code_length = static_cast<unsigned>(argc > 2
                                                ? std::atoi(argv[2])
                                                : 100);
  p.env.generations = static_cast<unsigned>(argc > 3
                                            ? std::atoi(argv[3])
                                            : 100);

  symbol_factory factory;
  p.sset.insert<X>();
  p.sset.insert<Y>();
  p.sset.insert<Z>();
  p.sset.insert(factory.make("FADD"));
  p.sset.insert(factory.make("FSUB"));
  p.sset.insert(factory.make("FMUL"));
  p.sset.insert(factory.make("FIFL"));
  p.sset.insert(factory.make("FIFE"));

  auto eva(std::make_unique<my_evaluator>());

  evolution<i_mep, std_es> evo(p, *eva);

  evo.run(1);
}
