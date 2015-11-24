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

#if !defined(FOREX_TRADING_DATA_H)
#define      FOREX_TRADING_DATA_H

// ****************************************************************************
// timepoint
// ****************************************************************************
struct timepoint
{
  unsigned year;
  unsigned month;
  unsigned day;
  unsigned hour;
  unsigned minute;
};

timepoint string_to_timepoint(const std::string &s)
{
  timepoint tf;

  std::sscanf(s.c_str(), "%2u.%2u.%4u %2u:%2u",
              &tf.day, &tf.month, &tf.year, &tf.hour, &tf.minute);

  return tf;
}

// ****************************************************************************
// trading_data
// ****************************************************************************
class trading_data
{
public:
  enum timeframe {short_tf = 0, medium_tf, long_tf, sup_tf};

  static constexpr unsigned ratio[sup_tf] =
  {
    1,   // Duration of timeframe short_t is one minute
    60,  // 60 short_t timeframes are equivalent to one medium_t (1 hour)
    24   // 24 medium_t timeframes are equivalent to one long_t (1 day)
  };

public:
  trading_data() { load_data("forex_files/eurusd_1m_bid.csv"); }

  bool empty() const { return trading[0].empty(); }

  unsigned bars(unsigned tf = 0) const
  { return static_cast<unsigned>(trading[tf].size()); }
  double close(unsigned tf, unsigned shift) const
  { return get(tf, shift).close; }
  double close(unsigned shift) const { return close(0, shift); }
  double high(unsigned tf, unsigned shift) const { return get(tf, shift).high; }
  double high(unsigned shift) const { return high(0, shift); }
  double low(unsigned tf, unsigned shift) const { return get(tf, shift).low; }
  double low(unsigned shift) const { return low(0, shift); }
  double open(unsigned tf, unsigned shift) const
  { return get(tf, shift).open; }
  double open(unsigned shift) const { return open(0, shift); }
  double volume(unsigned tf, unsigned shift) const
  { return get(tf, shift).volume; }
  double volume(unsigned shift) const { return volume(0, shift); }

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

  std::array<std::vector<trade_info_point>, sup_tf> trading;

private:
  bool compute_longer_timeframes();
  trade_info_point get(unsigned tf, unsigned shift) const
  {
    assert(tf < trading.size());
    assert(shift < bars(tf));
    return trading[tf][shift];
  }

  bool load_data(const std::string &);
  std::pair<double, double> minmax_vol(unsigned) const;
  bool normalize_volume(unsigned);
};

#endif  // include guard
