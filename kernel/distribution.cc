/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_NO_LIB)
#include "kernel/distribution.h"
#endif

namespace vita
{
  template<>
  VITA_INLINE
  void distribution<fitness_t>::add(fitness_t val)
  {
    if (!val.isnan())
    {
      if (!count)
      {
        min = max = val;

        delta_ = m2_ = mean = variance = fitness_t(0.0);
      }
      else if (val < min)
        min = val;
      else if (val > max)
        max = val;

      ++count;

      update_variance(val);

      for (unsigned i(0); i < fitness_t::size; ++i)
        val[i] = round_to(val[i]);
      ++freq[val];
    }
  }

  template<>
  VITA_INLINE
  fitness_t distribution<fitness_t>::standard_deviation() const
  {
    return variance.sqrt();
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  template<>
  VITA_INLINE
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

    if (variance.isnan() || variance < fitness_t(0.0))
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: negative variance."
                  << std::endl;
      return false;
    }

    return true;
  }
}  // namespace vita
