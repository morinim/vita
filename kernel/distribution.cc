/**
 *
 *  \file distribution.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "distribution.h"

namespace vita
{
  template<>
  void distribution<fitness_t>::clear()
  {
    count = 0;

    delta_ = m2_ = mean = variance = min = max = {};

    freq.clear();
  }

  template<>
  void distribution<fitness_t>::add(fitness_t val)
  {
    if (!val.isnan())
    {
      if (!count)
      {
        min = max = val;

        delta_ = m2_ = mean = variance = fitness_t(val.size(), 0.0);
      }
      else if (val < min)
        min = val;
      else if (val > max)
        max = val;

      ++count;

      update_variance(val);

      for (size_t i(0); i < val.size(); ++i)
        val[i] = round_to(val[i]);
      ++freq[val];
    }
  }

  template<>
  fitness_t distribution<fitness_t>::standard_deviation() const
  {
    return variance.sqrt();
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  template<>
  bool distribution<fitness_t>::debug(bool verbose) const
  {
    if (min.isfinite() && mean.isfinite() && min > mean)
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: min=" << min << " > mean="
                  << mean << "." << std::endl;
      return false;
    }

    if (max.isfinite() && mean.isfinite() && max < mean)
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: max=" << max << " < mean="
                  << mean << "." << std::endl;
      return false;
    }

    if (variance.isnan() || variance < fitness_t(variance.size(), 0.0))
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: negative variance."
                  << std::endl;
      return false;
    }

    return true;
  }
}  // namespace vita
