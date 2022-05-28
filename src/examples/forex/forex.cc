/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/vita.h"
#include "trade_simulator.h"

enum timeframe : unsigned {short_tf = 0, medium_tf, long_tf, sup_tf};

namespace fxs  // Forex symbols
{
using team = vita::team<vita::i_mep>;

enum
{
  c_logic,  // boolean values
  c_money,  // currencies
  c_volume  // volume of a transaction
};

///
/// \tparam TF timeframe
/// \tparam I  index of a candle. `0` is the current bar, `1` is the previous
///            one and so on (a greater value identifies an older candle).
///            It's called shift in Metatrader
///
template<timeframe TF, unsigned I>
class tfi_terminal : public vita::terminal
{
public:
  tfi_terminal(const std::string &n, vita::category_t c)
    : vita::terminal(n + "("
                     + std::to_string(TF) + "," + std::to_string(I) +
                     ")", c)
  {
  }

  bool input() const override { return true; }
  vita::value_t eval(vita::symbol_params &) const override { return {}; }

  std::string display(vita::terminal_param_t, format) const override
  {
    return name();
  }
};

template<timeframe TF, unsigned I>
struct close : tfi_terminal<TF, I>
{
  close() : tfi_terminal<TF, I>("close", c_money) {}
};

template<timeframe TF, unsigned I>
struct high : tfi_terminal<TF, I>
{
  high() : tfi_terminal<TF, I>("high", c_money) {}
};

template<timeframe TF, unsigned I>
struct low : tfi_terminal<TF, I>
{
  low() : tfi_terminal<TF, I>("low", c_money) {}
};

template<timeframe TF, unsigned I>
struct open : tfi_terminal<TF, I>
{
  open() : tfi_terminal<TF, I>("open", c_money) {}
};

/// Black candle is formed when the opening price is higher than the closing
/// price.
template<timeframe TF, unsigned I>
struct black_candle : tfi_terminal<TF, I>
{
  black_candle() : tfi_terminal<TF, I>("black_candle", c_logic) {}
};

/// White candle is formed when the opening price is lower than the closing
/// price.
template<timeframe TF, unsigned I>
struct white_candle : tfi_terminal<TF, I>
{
  white_candle() : tfi_terminal<TF, I>("white_candle", c_logic) {}
};

/// Doji are important candlesticks that provide information on their own and
/// as components of in a number of important patterns. Doji form when a
/// security's open and close are virtually equal.
/// The length of the upper and lower shadows can vary and the resulting
/// candlestick looks like a cross, inverted cross or plus sign. Alone, doji
/// are neutral patterns. Any bullish or bearish bias is based on preceding
/// price action and future confirmation.
template<timeframe TF, unsigned I>
struct doji : tfi_terminal<TF, I>
{
  doji() : tfi_terminal<TF, I>("doji", c_logic) {}
};

/// Bearish Harami (meaning "pregnant" in Japanese) consists of an unusually
/// large white body followed by a small black body (contained within large
/// white body).
/// It's considered as a bearish pattern when preceded by an uptrend.
template<timeframe TF>
struct bearish_harami : tfi_terminal<TF, 1>
{
  bearish_harami() : tfi_terminal<TF, 1>("bearish_harami", c_logic) {}
};

/// Bullish Harami (meaning "pregnant" in Japanese) consists of an unusually
/// large black body followed by a small white body (contained within large
/// black body).
/// It's considered as a bullish pattern when preceded by a downtrend.
template<timeframe TF>
struct bullish_harami : tfi_terminal<TF, 1>
{
  bullish_harami() : tfi_terminal<TF, 1>("bullish_harami", c_logic) {}
};

/// Dark Cloud Cover consists of a long white candlestick followed by a black
/// candlestick that opens above the high of the white candlestick and closes
/// well into the body of the white candlestick. It is considered as a bearish
/// reversal signal during an uptrend.
template<timeframe TF>
struct dark_cloud_cover : tfi_terminal<TF, 1>
{
  dark_cloud_cover() : tfi_terminal<TF, 1>("dark_cloud_cover", c_logic) {}
};

template<timeframe TF, unsigned I>
struct long_candle : tfi_terminal<TF, I>
{
  long_candle() : tfi_terminal<TF, I>("long_candle", c_logic) {}
};

template<timeframe TF, unsigned I>
struct long_black_candle : tfi_terminal<TF, I>
{
  long_black_candle() : tfi_terminal<TF, I>("long_black_candle", c_logic) {}
};

template<timeframe TF, unsigned I>
struct long_white_candle : tfi_terminal<TF, I>
{
  long_white_candle() : tfi_terminal<TF, I>("long_white_candle", c_logic) {}
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

class evaluator : public vita::evaluator<team>
{
public:
  explicit evaluator(trade_simulator *ts) : ts_(ts) {}

  vita::fitness_t operator()(const team &t) final
  {
    return {ts_->run(t)};
  }

private:
  trade_simulator *ts_;
};

class search : public vita::search<team, vita::alps_es>
{
public:
  using vita::search<team, vita::alps_es>::search;
};

}  // namespace fxs

template<template<timeframe, unsigned> class F>
void insert_symbol(vita::symbol_set *ss)
{
  ss->insert<F<short_tf, 1>>();
  ss->insert<F<short_tf, 2>>();
  ss->insert<F<short_tf, 3>>();

  ss->insert<F<medium_tf, 1>>();
  ss->insert<F<medium_tf, 2>>();
  ss->insert<F<medium_tf, 3>>();

  ss->insert<F<long_tf, 1>>();
  ss->insert<F<long_tf, 2>>();
  ss->insert<F<long_tf, 3>>();
}

template<template<timeframe> class F>
void insert_symbol(vita::symbol_set *ss)
{
  ss->insert<F< short_tf>>();
  ss->insert<F<medium_tf>>();
  ss->insert<F<  long_tf>>();
}

bool setup_symbols(vita::symbol_set *ss)
{
  insert_symbol<fxs::close>(ss);
  insert_symbol<fxs::high>(ss);
  insert_symbol<fxs::low>(ss);
  insert_symbol<fxs::open>(ss);

  insert_symbol<fxs::black_candle>(ss);
  insert_symbol<fxs::white_candle>(ss);

  insert_symbol<fxs::long_candle>(ss);
  insert_symbol<fxs::long_black_candle>(ss);
  insert_symbol<fxs::long_white_candle>(ss);

  insert_symbol<fxs::doji>(ss);

  insert_symbol<fxs::bearish_harami>(ss);
  insert_symbol<fxs::bullish_harami>(ss);
  insert_symbol<fxs::dark_cloud_cover>(ss);

  ss->insert<fxs::l_and>();
  ss->insert< fxs::l_or>();

  ss->insert<fxs::add>();
  ss->insert<fxs::sub>();

  ss->insert<fxs::lt_m>();

  return true;
}

int main()
{
  vita::problem p;

  if (!setup_symbols(&p.sset))
    return EXIT_FAILURE;

  p.env.individuals = 30;
  p.env.min_individuals = 8;
  p.env.mep.code_length = 200;
  p.env.generations = 400;
  p.env.layers = 6;
  p.env.team.individuals = 2;  // DO NOT CHANGE
  p.env.alps.age_gap = 10;
  p.env.cache_size = 20;       // A hash table of `2^20` elements.
                               // Considering the speed of the Metatrader
                               // back-tester this should ensure a very low
                               // hash collision rate.

  p.env.stat.dynamic_file    = "dynamic.txt";
  p.env.stat.layers_file     = "layers.txt";
  p.env.stat.population_file = "population.txt";
  p.env.stat.summary_file    = "summary.txt";
  p.env.stat.ind_format = vita::out::mql_language_f;

  p.env.misc.serialization_file = "cache.txt";

  fxs::search engine(p);

  trade_simulator ts;
  engine.training_evaluator<fxs::evaluator>(&ts);

  std::cout << "STARTING RUN\n";
  engine.run(1);
}
