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

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "kernel/utility.h"

struct timepoint
{
  unsigned year;
  unsigned month;
  unsigned day;
  unsigned hour;
  unsigned minute;
};

namespace timeframe
{
enum duration {short_t = 0, medium_t, long_t, sup};

constexpr unsigned ratio[sup] =
{
  1,   // Duration of timeframe short_t is one minute
  60,  // 60 short_t timeframes are equivalent to one medium_t (1 hour)
  24   // 24 medium_t timeframes are equivalent to one long_t (1 day)
};
}

class trading_data
{
public:
  trading_data() { load_data("eurusd_1m_bid.csv"); }

  bool empty() const { return trading[0].empty(); }

  int bars(int tf = 0) const { return static_cast<int>(trading[tf].size()); }
  double close(int tf, int shift) const { return get(tf, shift).close; }
  double close(int shift) const { return close(0, shift); }
  double high(int tf, int shift) const { return get(tf, shift).high; }
  double high(int shift) const { return high(0, shift); }
  double low(int tf, int shift) const { return get(tf, shift).low; }
  double low(int shift) const { return low(0, shift); }
  double open(int tf, int shift) const { return get(tf, shift).open; }
  double open(int shift) const { return open(0, shift); }
  double volume(int tf, int shift) const { return get(tf, shift).volume; }
  double volume(int shift) const { return volume(0, shift); }

private:
  struct trade_info_point
  {
    explicit trade_info_point(double o = 0.0, double h = 0.0, double l = 0.0,
                              double c = 0.0, double v = 0.0)
      : open(o), high(h), low(l), close(c), volume(v)
    {
      assert(h >= o);
      assert(h >= l);
      assert(h >= c);
      assert(l <= o);
      assert(l <= c);
      assert(v > 0.0);
    }

    // timepoint point;
    double   open;
    double   high;
    double    low;
    double  close;
    double volume;
  };

  std::array<std::vector<trade_info_point>, timeframe::sup> trading;

private:
  bool compute_longer_timeframes();
  trade_info_point get(int tf, int shift) const
  {
    assert(tf < static_cast<int>(trading.size()));
    assert(shift < bars(tf));
    return trading[tf][shift];
  }

  bool load_data(const std::string &);
  std::pair<double, double> minmax_vol(int) const;
  bool normalize_volume(int);
};

struct order
{
  enum {na = -1, buy = 0, sell};

  int type = na;
  double amount = 0.0;
  int bar = -1;
};

// More details:
// - https://en.wikipedia.org/wiki/Currency_pair
class trade_simulator
{
public:
  trade_simulator() {}

  // The balance, expressed in counter currency, not including the floating
  // profit/loss from any open trades.
  // Transactions in forex markets are quoted as pairs because traders buy one
  // currency and sell another. For example, EUR/USD is presented as the price
  // of how many US dollars should be spent to buy a Euro.
  // The **base currency** is located to the left and the **counter currency**
  // is located to the right.
  double account_balance() const { return balance_; }

  // If the current bid price for the EUR/USD currency pair is 1.5760 this
  // means that you can sell EUR/USD at 1.5760
  double bid() const { return td_.open(cur_bar_); }

  // If the current ask price for the EUR/USD currency pair is 1.5763 this
  // means that you can buy EUR/USD at 1.5763
  double ask() const { return bid() + spread_; }

  double order_amount() const { return order_.amount; }
  int order_type() const { return order_.type; }

  bool order_send(int, double);
  bool close();

private:
  double spread_ = 0.0002;
  double balance_ = 0.0;

  order order_;
  int cur_bar_ = 0;

  trading_data td_;
};

timepoint string_to_timepoint(const std::string &s)
{
  timepoint tf;

  std::sscanf(s.c_str(), "%2u.%2u.%4u %2u:%2u",
              &tf.day, &tf.month, &tf.year, &tf.hour, &tf.minute);

  return tf;
}

std::pair<double, double> trading_data::minmax_vol(int tf) const
{
  double min(999999999.0), max(0.0);
  for (const auto &tip : trading[tf])
  {
    if (tip.volume < min)
      min = tip.volume;
    else if (tip.volume > max)
      max = tip.volume;
  }

  assert(min < max);

  return {min, max};
}

bool trading_data::normalize_volume(int tf)
{
  std::cout << "  Normalizing volumes for timeframe " << tf << '\n';

  std::cout << "  Getting min/max volumes\r" << std::flush;
  auto minmax(minmax_vol(tf));

  std::cout << "  Scaling                \r" << std::flush;
  const double delta(minmax.second - minmax.first);
  for (auto &tip : trading[tf])
    tip.volume = (tip.volume - minmax.first) / delta;

  return true;
}

bool trading_data::compute_longer_timeframes()
{
  std::cout << "COMPUTING RESUMES FOR LONGER TIMEFRAMES\n";

  normalize_volume(0);

  for (int tf(1); tf < timeframe::sup; ++tf)
  {
    double frame_high(high(tf - 1, 0)), frame_low(low(tf - 1, 0));
    double frame_volume(0.0);

    int begin(0), end(timeframe::ratio[tf]);
    for (int i(0); i < bars(tf - 1); ++i)
    {
      if (i == end || i + 1 == bars(tf - 1))
      {
        trading[tf].emplace_back(open(tf - 1, begin), frame_high, frame_low,
                                 close(tf - 1, i - 1), frame_volume);

        frame_high = high(tf - 1, i);
        frame_low = low(tf - 1, i);
        frame_volume = 0.0;

        begin = end;
        end += timeframe::ratio[tf];
      }

      frame_high = std::max(high(tf - 1, i), frame_high);
      frame_low = std::min(low(tf - 1, i), frame_low);
      frame_volume += volume(tf - 1, i);
    }

    normalize_volume(tf);
    std::cout << "  Timeframe " << tf << " computed (" << bars(tf)
              << " examples)\n";
  }

#if !defined(NDEBUG)
  auto minmax(minmax_vol(0));
  for (int tf(1); tf < timeframe::sup; ++tf)
  {
    auto mm(minmax_vol(tf));
    assert(minmax == mm);
  }
#endif

  return true;
}

bool trading_data::load_data(const std::string &filename)
{
  std::cout << "READING DATA\n";

  using vita::csv_parser;

  std::ifstream from(filename);
  if (!from)
  {
    std::cerr << "Error opening input file\n";
    return false;
  }

  enum {f_timestamp = 0, f_open, f_high, f_low, f_close, f_volume};

  // Skips null volume records.
  // We should check for holidays and skip the corrensponding records but this
  // is simpler and almost equivalent for timeframes greater than 10s.
  auto csv_filter = [](vita::csv_parser::record_t *r)
  {
    return std::stoi((*r)[f_volume]) > 0;
  };

  int i(0);
  for (auto record : csv_parser(from).filter_hook(csv_filter))
  {
    trading[0].emplace_back(std::stod(record[  f_open]),
                            std::stod(record[  f_high]),
                            std::stod(record[   f_low]),
                            std::stod(record[ f_close]),
                            std::stod(record[f_volume]));

    if (++i % 100000 == 0)
      std::cout << "  " << i << '\r' << std::flush;
  }
  std::cout << "  " << i << " examples read\n";

  return compute_longer_timeframes();
}

// A lot is the basic trade size. It translates to 100000 units of the base
// currency (the currency on the left of the currancy pair).
// Also used are mini lot (10000 units) and micro lots (1000 units).
bool trade_simulator::order_send(int type, double lots)
{
  assert(type == order::buy || type == order::sell);
  assert(lots >= 0.01);

  const double amount(lots * 100000);
  switch (type)
  {
  case order::buy:
    // Buying base currency.
    balance_ -= amount * ask();
    break;

  case order::sell:
    // Selling base currency to buy counter currency.
    balance_ += amount * bid();
    break;

  default:
    return false;
  }

  order_.type = type;
  order_.amount = amount;
  order_.bar = cur_bar_;

  return true;
}

bool trade_simulator::close()
{
  switch (order_.type)
  {
  case order::buy:
    // Having bought base currency, we now want back counter currency.
    balance_ += order_amount() * bid();
    break;

  case order::sell:
    // Having sold base currency to buy counter currency, we now want back
    // base currency.
    balance_ -= order_amount() * ask();
    break;

  default:
    return false;
  };

  order_ = order();
  return true;
}

int main()
{
  trading_data data;

  if (data.empty())
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
