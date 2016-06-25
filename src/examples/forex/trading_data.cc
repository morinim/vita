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

#include "trading_data.h"

#include "utility/csv_parser.h"
#include "utility/timer.h"

namespace
{

std::tm string_to_timepoint(const std::string &s)
{
  std::tm tp;

  std::sscanf(s.c_str(), "%2d.%2d.%4d %2d:%2d",
              &tp.tm_mday, &tp.tm_mon, &tp.tm_year, &tp.tm_hour, &tp.tm_min);

  assert(tp.tm_year >= 1900);
  tp.tm_year -= 1900;

  assert(tp.tm_mon >= 1);
  --tp.tm_mon;

  tp.tm_sec = 0;
  tp.tm_isdst = -1;  // DST (Daylight Saving Time). A negative value causes the
                     //  mktime() function to attempt to divine whether summer
                     // time is in effect for the specified time
  return tp;
}

const std::string tf_name[timeframe::sup_tf + 1] = {"Short", "Medium", "Long",
                                                    "Sup"};
}  // namespace

trading_data::trading_data(const std::string &fn)
{
  load_data(fn);
}

bool trading_data::load_data(const std::string &filename)
{
  std::cout << "READING DATA\n";
  vita::timer t;

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
  auto csv_filter = [](vita::csv_parser::record_t &r)
  {
    return std::stod(r[f_volume]) > 0.0;
  };

  unsigned i(0);
  std::tm p[2];
  for (auto record : csv_parser(from).filter_hook(csv_filter))
  {
    trading[short_tf].emplace_back(std::stod(record[  f_open]),
                                   std::stod(record[  f_high]),
                                   std::stod(record[   f_low]),
                                   std::stod(record[ f_close]),
                                   std::stod(record[f_volume]));

    if (i < 2)
      p[i] = string_to_timepoint(record[f_timestamp]);

    if (++i % 100000 == 0)
      std::cout << "  " << i << '\r' << std::flush;
  }

  assert(i > 2);
  std::cout << "  " << i << " records read ("
            << std::chrono::duration_cast<std::chrono::seconds>(
                 t.elapsed()).count()
            << "s)\n";

  if (!set_timeframe_duration(p[0], p[1]) ||
      !compute_longer_timeframes())
    return false;

  return debug();
}

bool trading_data::set_timeframe_duration(std::tm p0, std::tm p1)
{
  auto t0(std::mktime(&p0));
  auto t1(std::mktime(&p1));
  Expects(t1 > t0);
  const auto delta_s(std::difftime(t1, t0));
  std::chrono::seconds sec(static_cast<std::uint64_t>(delta_s));

  assert(sec > std::chrono::seconds::zero());
  assert(sec <= std::chrono::hours(4));

  tf_duration[short_tf] = sec;

  if (sec <= std::chrono::minutes(5))
  {
    tf_duration[medium_tf] = std::chrono::minutes(30);
    tf_duration[long_tf]   = std::chrono::hours(1);
  }
  else if (sec <= std::chrono::minutes(30))
  {
    tf_duration[medium_tf] = std::chrono::hours(1);
    tf_duration[long_tf]   = std::chrono::hours(4);
  }
  else if (sec <= std::chrono::hours(1))
  {
    tf_duration[medium_tf] =  std::chrono::hours(4);
    tf_duration[long_tf]   = std::chrono::hours(24);
  }
  else if (sec <= std::chrono::hours(4))
  {
    tf_duration[medium_tf] =  std::chrono::hours(24);
    tf_duration[long_tf]   = std::chrono::hours(168);  // 1 week
  }

  for (timeframe tf(short_tf); tf < sup_tf; tf = longer(tf))
    std::cout << "  " << tf_name[vita::as_integer(tf)]
              << " timeframe duration is " << tf_duration[tf].count() << "s\n";

  return true;
}

std::pair<double, double> trading_data::minmax_vol(timeframe tf) const
{
  const auto &dataset(trading[vita::as_integer(tf)]);

  const auto res(std::minmax_element(dataset.begin(), dataset.end(),
                                     [](const trade_info_point &t1,
                                        const trade_info_point &t2)
                                     {
                                       return t1.volume < t2.volume;
                                     }));

  return {res.first->volume, res.second->volume};
}

bool trading_data::normalize_volume(timeframe tf)
{
  std::cout << "  Normalizing volumes for " << tf_name[vita::as_integer(tf)]
            << " timeframe\n";

  std::cout << "  Getting min/max volumes\r" << std::flush;
  auto minmax(minmax_vol(tf));

  std::cout << "  Scaling                \r" << std::flush;
  const double delta(minmax.second - minmax.first);
  for (auto &tip : trading[tf])
  {
    tip.volume = (tip.volume - minmax.first) / delta;

    assert(0.0 <= tip.volume);
    assert(tip.volume <= 1.0);
  }

  return true;
}

bool trading_data::compute_longer_timeframes()
{
  std::cout << "COMPUTING RESUMES FOR LONGER TIMEFRAMES\n";

  for (timeframe tf(short_tf); tf < long_tf; tf = longer(tf))
  {
    double frame_high(high(tf, 0)), frame_low(low(tf, 0));
    double frame_volume(0.0);

    const auto ratio(static_cast<std::size_t>(tf_duration[longer(tf)].count() /
                                              tf_duration[tf].count()));
    std::size_t begin(0), end(ratio);
    for (std::size_t i(0); i < bars(tf); ++i)
    {
      if (i == end || i + 1 == bars(tf))
      {
        trading[longer(tf)].emplace_back(open(tf, begin), frame_high,
                                         frame_low, close(tf, i - 1),
                                         frame_volume);

        frame_high = high(tf, i);
        frame_low = low(tf, i);
        frame_volume = 0.0;

        begin = end;
        end += ratio;
      }

      frame_high = std::max(high(tf, i), frame_high);
      frame_low = std::min(low(tf, i), frame_low);
      frame_volume += volume(tf, i);
    }

    std::cout << "  " << tf_name[longer(tf)] << " timeframe computed ("
              << bars(longer(tf)) << " examples)\n";
  }

  for (timeframe tf(short_tf); tf < sup_tf; tf = longer(tf))
    normalize_volume(tf);

#if !defined(NDEBUG)
  auto minmax(minmax_vol(short_tf));
  for (timeframe tf(medium_tf); tf < sup_tf; tf = longer(tf))
  {
    auto mm(minmax_vol(tf));
    assert(minmax == mm);
  }
#endif

  return true;
}

bool trading_data::empty() const
{
  return trading[short_tf].empty();
}

bool trading_data::debug() const
{
  bool ret(true);

  double prev(open(short_tf, 0));
  for (std::size_t i(1); i < bars(); ++i)
  {
    if (std::fabs(prev - open(short_tf, i)) > 0.1)
    {
      std::cerr << "Open(" << i - 1 << ") - Open(" << i << ") = "
                << (prev - open(short_tf, i)) << "\n";
      ret = false;
    }

    prev = open(short_tf, i);
  }

  return ret;
}

bool black_candle(const trading_data &d, timeframe tf, std::size_t i)
{
  return d.close(tf, i) < d.open(tf, i);
}

bool white_candle(const trading_data &d, timeframe tf, std::size_t i)
{
  return d.close(tf, i) > d.open(tf, i);
}

/// \param[in] d trading data.
/// \param[in] tf timeframe.
/// \param[in] i candle (array mode: 0 is the older candle in the dataset).
/// \return `true` if the i-th bar is a long candle.
/// \note
/// Our definition of long candle: a candle whose real body is at least 3
/// times the average real body of the last 5 candles.
bool long_candle(const trading_data &d, timeframe tf, std::size_t i)
{
  if (i < 5)
    return false;

  double avg_length(0.0);
  for (std::size_t j(i > 5 ? i - 5 : 0); j != i; ++j)
    avg_length += std::fabs(d.open(tf, j) - d.close(tf, j));

  avg_length /= 5.0;

  return std::fabs(d.open(tf, i) - d.close(tf, i)) > 3.0 * avg_length;
}

bool long_black_candle(const trading_data &d, timeframe tf, std::size_t i)
{
  return black_candle(d, tf, i) && long_candle(d, tf, i);
}

bool long_white_candle(const trading_data &d, timeframe tf, std::size_t i)
{
  return white_candle(d, tf, i) && long_candle(d, tf, i);
}
