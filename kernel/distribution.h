/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DISTRIBUTION_H)
#define      VITA_DISTRIBUTION_H

#include <cmath>
#include <map>

#include "kernel/fitness.h"

namespace vita
{
  ///
  /// \a distribution \c class simplify the calculation of statistics regarding
  /// a sequence (mean, variance, standard deviation, entropy, min and max).
  ///
  template<class T = fitness_t>
  class distribution
  {
  public:
    distribution();

    void clear();

    void add(T);

    T standard_deviation() const;
    double entropy() const;

    bool debug(bool) const;

    std::uintmax_t count;

    T     mean;
    T variance;
    T      min;
    T      max;

    std::map<T, std::uintmax_t> freq;

  private:
    void update_variance(T);

    T delta_;
    T    m2_;
  };

  ///
  /// Just the initial setup.
  ///
  template<class T>
  distribution<T>::distribution()
  {
    clear();
  }

  ///
  /// Resets gathered statics.
  ///
  template<class T>
  void distribution<T>::clear()
  {
    count = 0;

    delta_ = m2_ = mean = variance = min = max = 0.0;

    freq.clear();
  }

  ///
  /// \param val a value to be rounded.
  ///
  /// \brief Rounds \a val to the number of decimals of \c float_epsilon.
  ///
  template<class T>
  T round_to(T val)
  {
    constexpr T float_epsilon(0.0001);

    val /= float_epsilon;
    val = std::round(val);
    val *= float_epsilon;

    return val;
  }

  ///
  /// \param[in] val new value upon which statistics are recalculated.
  ///
  /// Add a new value to the distribution.
  ///
  template<class T>
  void distribution<T>::add(T val)
  {
    if (!std::isnan(val))
    {
      if (!count)
        min = max = val;
      else if (val < min)
        min = val;
      else if (val > max)
        max = val;

      ++count;
      ++freq[round_to(val)];

      update_variance(val);
    }
  }

  ///
  /// \return the entropy of the distribution.
  ///
  /// \f$H(X)=-\sum_{i=1}^n p(x_i) \dot log_b(p(x_i))\f$
  /// We use an offline algorithm
  /// (http://en.wikipedia.org/wiki/Online_algorithm).
  ///
  template<class T>
  double distribution<T>::entropy() const
  {
    const double c(1.0 / std::log(2.0));

    double h(0.0);
    for (const auto &f : freq)  // f.first: fitness, f.second: frequency
    {
      const double p(static_cast<double>(f.second) /
                     static_cast<double>(count));

      h -= p * std::log(p) * c;
    }

    return h;
  }

  ///
  /// \param[in] val new value upon which statistics are recalculated.
  ///
  /// Calculate running variance and cumulative average of a set. The
  /// algorithm used is due to Knuth (Donald E. Knuth - The Art of Computer
  /// Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232.
  /// Addison-Wesley).
  ///
  /// \see
  /// * <http://en.wikipedia.org/wiki/Online_algorithm>
  /// * <http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average>
  ///
  template<class T>
  void distribution<T>::update_variance(T val)
  {
    delta_ = val - mean;
    mean += delta_ / count;

    // This expression uses the new value of mean.
    m2_ += delta_ * (val - mean);

    variance = m2_ / count;
  }

  ///
  /// \return the standard deviation of the distribution.
  ///
  template<class T>
  T distribution<T>::standard_deviation() const
  {
    return std::sqrt(variance);
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  template<class T>
  bool distribution<T>::debug(bool verbose) const
  {
    if (std::isfinite(min) && std::isfinite(mean) && min > mean)
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: min=" << min << " > mean="
                  << mean << "." << std::endl;
      return false;
    }

    if (std::isfinite(max) && std::isfinite(mean) && max < mean)
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: max=" << max << " < mean="
                  << mean << "." << std::endl;
      return false;
    }

    if (std::isnan(variance) || variance < 0.0)
    {
      if (verbose)
        std::cerr << k_s_debug << " Distribution: negative variance."
                  << std::endl;
      return false;
    }

    return true;
  }

  template<> void distribution<fitness_t>::clear();
  template<> void distribution<fitness_t>::add(fitness_t);
  template<> fitness_t distribution<fitness_t>::standard_deviation() const;
  template<> bool distribution<fitness_t>::debug(bool) const;
}  // namespace vita

#endif  // Include guard
