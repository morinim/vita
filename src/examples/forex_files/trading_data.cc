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

#include "trading_data.h"

constexpr unsigned trading_data::ratio[sup_tf];

std::pair<double, double> trading_data::minmax_vol(unsigned tf) const
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

bool trading_data::normalize_volume(unsigned tf)
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

  for (unsigned tf(1); tf < sup_tf; ++tf)
  {
    double frame_high(high(tf - 1, 0)), frame_low(low(tf - 1, 0));
    double frame_volume(0.0);

    unsigned begin(0), end(ratio[tf]);
    for (unsigned i(0); i < bars(tf - 1); ++i)
    {
      if (i == end || i + 1 == bars(tf - 1))
      {
        trading[tf].emplace_back(open(tf - 1, begin), frame_high, frame_low,
                                 close(tf - 1, i - 1), frame_volume);

        frame_high = high(tf - 1, i);
        frame_low = low(tf - 1, i);
        frame_volume = 0.0;

        begin = end;
        end += ratio[tf];
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
  for (unsigned tf(1); tf < sup_tf; ++tf)
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

  unsigned i(0);
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
