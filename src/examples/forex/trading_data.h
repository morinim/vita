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

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "kernel/vita.h"

#if !defined(FOREX_TRADING_DATA_H)
#define      FOREX_TRADING_DATA_H

enum timeframe : unsigned {short_tf = 0, medium_tf, long_tf, sup_tf};

constexpr timeframe shorter(timeframe tf)
{ return timeframe(vita::as_integer(tf) - 1); }

constexpr timeframe longer(timeframe tf)
{ return timeframe(vita::as_integer(tf) + 1); }

// ****************************************************************************
// trading_data
// ****************************************************************************
class trading_data
{
public:
  std::chrono::seconds tf_duration[sup_tf];

  explicit trading_data(const std::string &);

  bool empty() const;

  std::size_t bars(timeframe tf = short_tf) const {return trading[tf].size();}

  double close(timeframe tf, std::size_t i) const { return get(tf, i).close; }
  double high(timeframe tf, std::size_t i) const { return get(tf, i).high; }
  double low(timeframe tf, std::size_t i) const { return get(tf, i).low; }
  double open(timeframe tf, std::size_t i) const { return get(tf, i).open; }
  double volume(timeframe tf, std::size_t i) const {return get(tf, i).volume;}

private:
  struct trade_info_point
  {
    explicit trade_info_point(double o = 0.0, double h = 0.0, double l = 0.0,
                              double c = 0.0, double v = 0.0)
      : open(o), high(h), low(l), close(c), volume(v)
    {
      Expects(h >= std::max(o, c));
      Expects(l <= std::min(o, c));
      Expects(v >= 0.0);
    }

    // std::tm point;
    double   open;
    double   high;
    double    low;
    double  close;
    double volume;
  };  // trade_info_point

  bool compute_longer_timeframes();
  bool debug() const;

  trade_info_point get(timeframe tf, std::size_t i) const
  {
    Expects(tf < sup_tf);
    Expects(i < bars(tf));

    return trading[tf][i];
  }

  bool load_data(const std::string &);
  std::pair<double, double> minmax_vol(timeframe) const;
  bool normalize_volume(timeframe);
  bool set_timeframe_duration(std::tm, std::tm);

  std::array<std::vector<trade_info_point>, sup_tf> trading;
};

bool black_candle(const trading_data &, timeframe, std::size_t);
bool white_candle(const trading_data &, timeframe, std::size_t);
bool long_candle(const trading_data &, timeframe, std::size_t);
bool long_black_candle(const trading_data &, timeframe, std::size_t);
bool long_white_candle(const trading_data &, timeframe, std::size_t);

#endif  // include guard
