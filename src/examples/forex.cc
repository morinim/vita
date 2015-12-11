/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/evaluator.h"
#include "kernel/interpreter.h"
#include "kernel/i_mep.h"
#include "kernel/search.h"
#include "kernel/src/primitive/bool.h"
#include "kernel/src/primitive/real.h"
#include "kernel/team.h"
#include "kernel/utility.h"

#include "forex_files/trading_data.cc"
#include "forex_files/trade_simulator.cc"

namespace fxs  // Forex symbols
{
using i_interp = vita::core_interpreter;

enum {c_logic, c_money, c_volume};

template<unsigned TF, unsigned I>
class close : public vita::terminal
{
public:
  explicit close(trade_simulator *ts)
    : vita::terminal("CLOSE[" + std::to_string(TF) + "," + std::to_string(I) +
                     "]", c_money),
      ts_(ts)
  {}

  virtual bool input() const override { return true; }
  virtual vita::any eval(i_interp *) const override
  { return vita::any(ts_->close(TF, I)); }

private:
  trade_simulator *ts_;
};

template<unsigned TF, unsigned I>
class high : public vita::terminal
{
public:
  explicit high(trade_simulator *ts)
    : vita::terminal("HIGH[" + std::to_string(TF) + "," + std::to_string(I) +
                     "]", c_money),
      ts_(ts)
  {}

  virtual bool input() const override { return true; }
  virtual vita::any eval(i_interp *) const override
  { return vita::any(ts_->high(TF, I)); }

private:
  trade_simulator *ts_;
};

template<unsigned TF, unsigned I>
class low : public vita::terminal
{
public:
  explicit low(trade_simulator *ts)
    : vita::terminal("LOW[" + std::to_string(TF) + "," + std::to_string(I) +
                     "]", c_money),
      ts_(ts)
  {}

  virtual bool input() const override { return true; }
  virtual vita::any eval(i_interp *) const override
  { return vita::any(ts_->low(TF, I)); }

private:
  trade_simulator *ts_;
};

template<unsigned TF, unsigned I>
class open : public vita::terminal
{
public:
  explicit open(trade_simulator *ts)
    : vita::terminal("OPEN[" + std::to_string(TF) + "," + std::to_string(I) +
                     "]", c_money),
      ts_(ts)
  {}

  virtual bool input() const override { return true; }
  virtual vita::any eval(i_interp *) const override
  { return vita::any(ts_->open(TF, I)); }

private:
  trade_simulator *ts_;
};

template<unsigned TF, unsigned I>
class volume : public vita::terminal
{
public:
  explicit volume(trade_simulator *ts)
    : vita::terminal("VOLUME[" + std::to_string(TF) + "," + std::to_string(I) +
                     "]", c_volume),
      ts_(ts)
  {}
  virtual bool input() const override { return true; }
  virtual vita::any eval(i_interp *) const override
  { return vita::any(ts_->volume(TF, I)); }

private:
  trade_simulator *ts_;
};

struct l_and : vita::boolean::l_and
{
  l_and() : vita::boolean::l_and({c_logic}) {}
};

struct l_or : vita::boolean::l_or
{
  l_or() : vita::boolean::l_or({c_logic}) {}
};

struct one : vita::boolean::one
{
  one() : vita::boolean::one({c_logic}) {}
};

struct zero : vita::boolean::zero
{
  zero() : vita::boolean::zero({c_logic}) {}
};

struct add : vita::real::add
{
  add() : vita::real::add({c_money}) {}
};

struct sub : vita::real::sub
{
  sub() : vita::real::sub({c_money}) {}
};

struct lt_m : vita::real::lt
{
  lt_m() : vita::real::lt({c_money, c_logic}) {}
};

struct lt_v : vita::real::lt
{
  lt_v() : vita::real::lt({c_volume, c_logic}) {}
};

template<class T>
class evaluator : public vita::evaluator<T>
{
public:
  explicit evaluator(trade_simulator *ts) : ts_(ts) {}

  virtual vita::fitness_t operator()(const T &t) override
  {
    return {ts_->run(t)};
  }

private:
  trade_simulator *ts_;
};

}  // namespace fxs

bool setup_symbols(vita::symbol_set *ss, trade_simulator *ts)
{
  ss->insert(vita::make_unique<fxs::close<0, 1>>(ts));
  ss->insert(vita::make_unique<fxs::close<0, 2>>(ts));
  ss->insert(vita::make_unique<fxs::close<0, 3>>(ts));
  ss->insert(vita::make_unique<fxs::high<0, 1>>(ts));
  ss->insert(vita::make_unique<fxs::high<0, 2>>(ts));
  ss->insert(vita::make_unique<fxs::high<0, 3>>(ts));
  ss->insert(vita::make_unique<fxs::low<0, 1>>(ts));
  ss->insert(vita::make_unique<fxs::low<0, 2>>(ts));
  ss->insert(vita::make_unique<fxs::low<0, 3>>(ts));
  ss->insert(vita::make_unique<fxs::open<0, 1>>(ts));
  ss->insert(vita::make_unique<fxs::open<0, 2>>(ts));
  ss->insert(vita::make_unique<fxs::open<0, 3>>(ts));

  ss->insert(vita::make_unique<fxs::close<1, 1>>(ts));
  ss->insert(vita::make_unique<fxs::close<1, 2>>(ts));
  ss->insert(vita::make_unique<fxs::close<1, 3>>(ts));
  ss->insert(vita::make_unique<fxs::high<1, 1>>(ts));
  ss->insert(vita::make_unique<fxs::high<1, 2>>(ts));
  ss->insert(vita::make_unique<fxs::high<1, 3>>(ts));
  ss->insert(vita::make_unique<fxs::low<1, 1>>(ts));
  ss->insert(vita::make_unique<fxs::low<1, 2>>(ts));
  ss->insert(vita::make_unique<fxs::low<1, 3>>(ts));
  ss->insert(vita::make_unique<fxs::open<1, 1>>(ts));
  ss->insert(vita::make_unique<fxs::open<1, 2>>(ts));
  ss->insert(vita::make_unique<fxs::open<1, 3>>(ts));

  ss->insert(vita::make_unique<fxs::close<2, 1>>(ts));
  ss->insert(vita::make_unique<fxs::close<2, 2>>(ts));
  ss->insert(vita::make_unique<fxs::close<2, 3>>(ts));
  ss->insert(vita::make_unique<fxs::high<2, 1>>(ts));
  ss->insert(vita::make_unique<fxs::high<2, 2>>(ts));
  ss->insert(vita::make_unique<fxs::high<2, 3>>(ts));
  ss->insert(vita::make_unique<fxs::low<2, 1>>(ts));
  ss->insert(vita::make_unique<fxs::low<2, 2>>(ts));
  ss->insert(vita::make_unique<fxs::low<2, 3>>(ts));
  ss->insert(vita::make_unique<fxs::open<2, 1>>(ts));
  ss->insert(vita::make_unique<fxs::open<2, 2>>(ts));
  ss->insert(vita::make_unique<fxs::open<2, 3>>(ts));

  ss->insert(vita::make_unique<fxs::l_and>());
  ss->insert(vita::make_unique<fxs::l_or>());
  ss->insert(vita::make_unique<fxs::one>());
  ss->insert(vita::make_unique<fxs::zero>());

  ss->insert(vita::make_unique<fxs::add>());
  ss->insert(vita::make_unique<fxs::sub>());

  ss->insert(vita::make_unique<fxs::lt_m>());

  return true;
}

int main()
{
  trading_data td;
  if (td.empty())
    return EXIT_FAILURE;

  trade_simulator ts(td);

  vita::problem p(true);

  if (!setup_symbols(&p.sset, &ts))
    return EXIT_FAILURE;

  p.env.individuals = 40;
  p.env.min_individuals =  8;
  p.env.code_length = 200;
  p.env.generations = 400;
  p.env.layers = 6;
  p.env.team.individuals = 2;
  p.env.alps.age_gap = 10;
  p.env.stat.dynamic = true;
  p.env.stat.layers = true;
  p.env.stat.population = true;
  p.env.stat.summary = true;
  p.env.stat.dir = "forex_results/";

  std::cout << "STARTING RUN\n";
  vita::search<vita::team<vita::i_mep>, vita::alps_es> engine(p);

  using team = vita::team<vita::i_mep>;
  engine.set_evaluator(vita::make_unique<fxs::evaluator<team>>(&ts));

  engine.run(1);

  return EXIT_SUCCESS;
}
