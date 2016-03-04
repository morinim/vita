/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2016 EOS di Manlio Morini.
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
#include "utility/utility.h"

#include "forex_files/trading_data.cc"
#include "forex_files/trade_simulator.cc"

namespace fxs  // Forex symbols
{
using i_interp = vita::core_interpreter;

enum
{
  c_logic,  // boolean values
  c_money,  // currencies
  c_volume  // volume of a transaction
};

template<timeframe TF, unsigned I>
class tfi_terminal : public vita::terminal
{
public:
  tfi_terminal(const std::string &n, vita::category_t c, trade_simulator *ts)
    : vita::terminal(n + "[" + std::to_string(TF) + "," + std::to_string(I)
                     + "]", c),
      ts_(ts)
  {
    assert(ts_);
  }

  virtual bool input() const override { return true; }

protected:
  trade_simulator *ts_;
};

template<timeframe TF, unsigned I>
struct close : tfi_terminal<TF, I>
{
  explicit close(trade_simulator *ts)
    : tfi_terminal<TF, I>("CLOSE", c_money, ts)
  {}

  virtual vita::any eval(i_interp *) const override
  { return vita::any(this->ts_->close(TF, I)); }
};

template<timeframe TF, unsigned I>
struct high : tfi_terminal<TF, I>
{
  explicit high(trade_simulator *ts)
    : tfi_terminal<TF, I>("HIGH", c_money, ts)
  {}

  virtual vita::any eval(i_interp *) const override
  { return vita::any(this->ts_->high(TF, I)); }
};

template<timeframe TF, unsigned I>
struct low : tfi_terminal<TF, I>
{
  explicit low(trade_simulator *ts)
    : tfi_terminal<TF, I>("LOW", c_money, ts)
  {}

  virtual vita::any eval(i_interp *) const override
  { return vita::any(this->ts_->low(TF, I)); }
};

template<timeframe TF, unsigned I>
struct open : tfi_terminal<TF, I>
{
  explicit open(trade_simulator *ts)
    : tfi_terminal<TF, I>("OPEN", c_money, ts)
  {}

  virtual vita::any eval(i_interp *) const override
  { return vita::any(this->ts_->open(TF, I)); }
};

template<timeframe TF, unsigned I>
struct volume : tfi_terminal<TF, I>
{
  explicit volume(trade_simulator *ts)
    : tfi_terminal<TF, I>("VOLUME", c_volume, ts)
  {}

  virtual vita::any eval(i_interp *) const override
  { return vita::any(this->ts_->volume(TF, I)); }
};

/// Black candle is formed when the opening price is higher than the closing
/// price.
template<timeframe TF, unsigned I>
struct black_candle : tfi_terminal<TF, I>
{
  explicit black_candle(trade_simulator *ts)
    : tfi_terminal<TF, I>("BLACK_CANDLE", c_logic, ts)
  {
  }

  virtual vita::any eval(i_interp *) const override
  {
    return vita::any(this->ts_->black_candle(TF, I));
  }
};

/// White candle is formed when the opening price is lower than the closing
/// price.
template<timeframe TF, unsigned I>
struct white_candle : tfi_terminal<TF, I>
{
  explicit white_candle(trade_simulator *ts)
    : tfi_terminal<TF, I>("WHITE_CANDLE", c_logic, ts)
  {
  }

  virtual vita::any eval(i_interp *) const override
  {
    return vita::any(this->ts_->white_candle(TF, I));
  }
};

struct l_and : vita::boolean::l_and
{
  l_and() : vita::boolean::l_and({c_logic}) {}
};

struct l_or : vita::boolean::l_or
{
  l_or() : vita::boolean::l_or({c_logic}) {}
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

/// Bearish Harami (meaning "pregnant" in Japanese) consists of an unusually
/// large white body followed by a small black body (contained within large
/// white body).
/// It's considered as a bearish pattern when preceded by an uptrend.
template<timeframe TF>
struct bearish_harami : tfi_terminal<TF, 1>
{
  explicit bearish_harami(trade_simulator *ts)
    : tfi_terminal<TF, 1>("BEARISH_HARAMI", c_logic, ts)
  {
  }

  virtual vita::any eval(i_interp *) const override
  {
    bool ret(this->ts_->white_candle(TF, 2) &&
             this->ts_->black_candle(TF, 1) &&
             this->ts_->close(TF, 1) > this->ts_->open(TF, 2) &&
             this->ts_->open(TF, 1) < this->ts_->close(TF, 2));

    return vita::any(ret);
  }
};

/// Bullish Harami (meaning "pregnant" in Japanese) consists of an unusually
/// large black body followed by a small white body (contained within large
/// black body).
/// It's considered as a bullish pattern when preceded by a downtrend.
template<timeframe TF>
struct bullish_harami : tfi_terminal<TF, 1>
{
  explicit bullish_harami(trade_simulator *ts)
    : tfi_terminal<TF, 1>("BULLISH_HARAMI", c_logic, ts)
  {
  }

  virtual vita::any eval(i_interp *) const override
  {
    bool ret(this->ts_->black_candle(TF, 2) &&
             this->ts_->white_candle(TF, 1) &&
             this->ts_->close(TF, 1) < this->ts_->open(TF, 2) &&
             this->ts_->open(TF, 1) > this->ts_->close(TF, 2));

    return vita::any(ret);
  }
};

/// Dark Cloud Cover consists of a long white candlestick followed by a black
/// candlestick that opens above the high of the white candlestick and closes
/// well into the body of the white candlestick. It is considered as a bearish
/// reversal signal during an uptrend.
template<timeframe TF>
struct dark_cloud_cover : tfi_terminal<TF, 1>
{
  explicit dark_cloud_cover(trade_simulator *ts)
    : tfi_terminal<TF, 1>("DARK_CLOUD_COVER", c_logic, ts)
  {
  }

  virtual vita::any eval(i_interp *) const override
  {
    bool ret(this->ts_->white_candle(TF, 2) &&
             this->ts_->black_candle(TF, 1) &&
             this->ts_->close(TF, 1) > this->ts_->open(TF, 2) &&
             this->ts_->open(TF, 1) > this->ts_->high(TF, 2));

    return vita::any(ret);
  }
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
  ss->insert(vita::make_unique<fxs::close<short_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::close<short_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::close<short_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::high<short_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::high<short_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::high<short_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::low<short_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::low<short_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::low<short_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::open<short_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::open<short_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::open<short_tf, 3>>(ts));

  ss->insert(vita::make_unique<fxs::close<medium_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::close<medium_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::close<medium_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::high<medium_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::high<medium_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::high<medium_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::low<medium_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::low<medium_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::low<medium_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::open<medium_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::open<medium_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::open<medium_tf, 3>>(ts));

  ss->insert(vita::make_unique<fxs::close<long_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::close<long_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::close<long_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::high<long_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::high<long_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::high<long_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::low<long_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::low<long_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::low<long_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::open<long_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::open<long_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::open<long_tf, 3>>(ts));

  ss->insert(vita::make_unique<fxs::l_and>());
  ss->insert(vita::make_unique<fxs::l_or>());
  ss->insert(vita::make_unique<fxs::black_candle<short_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::white_candle<short_tf, 1>>(ts));
  ss->insert(vita::make_unique<fxs::black_candle<short_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::white_candle<short_tf, 2>>(ts));
  ss->insert(vita::make_unique<fxs::black_candle<short_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::white_candle<short_tf, 3>>(ts));
  ss->insert(vita::make_unique<fxs::bearish_harami<short_tf>>(ts));
  ss->insert(vita::make_unique<fxs::bearish_harami<medium_tf>>(ts));
  ss->insert(vita::make_unique<fxs::bearish_harami<long_tf>>(ts));
  ss->insert(vita::make_unique<fxs::bullish_harami<short_tf>>(ts));
  ss->insert(vita::make_unique<fxs::bullish_harami<medium_tf>>(ts));
  ss->insert(vita::make_unique<fxs::bullish_harami<long_tf>>(ts));
  ss->insert(vita::make_unique<fxs::dark_cloud_cover<short_tf>>(ts));
  ss->insert(vita::make_unique<fxs::dark_cloud_cover<medium_tf>>(ts));
  ss->insert(vita::make_unique<fxs::dark_cloud_cover<long_tf>>(ts));

  ss->insert(vita::make_unique<fxs::add>());
  ss->insert(vita::make_unique<fxs::sub>());

  ss->insert(vita::make_unique<fxs::lt_m>());

  return true;
}

int main()
{
  trading_data td("forex_files/eurusd_1m_bid.csv");
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
