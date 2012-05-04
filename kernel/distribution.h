/**
 *
 *  \file distribution.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(DISTRIBUTION_H)
#define      DISTRIBUTION_H

#include <cmath>
#include <map>

#include "kernel/fitness.h"

namespace vita
{
  ///
  /// \a distribution \c class simplify the calculation of statistics regarding
  /// a sequence (mean, variance, standard deviation, entropy, min and max).
  ///
  template<class T>
  class distribution
  {
  public:
    distribution();

    void clear();

    void add(T);

    T standard_deviation() const;
    double entropy() const;

    bool check() const;

    boost::uint64_t count;

    T     mean;
    T variance;
    T      min;
    T      max;

  private:
    void update_variance(T);

    std::map<T, unsigned> freq_;

    T delta_;
    T    m2_;
  };


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

    freq_.clear();
  }

  ///
  /// \param[in] val new fitness upon which statistics are recalculated.
  ///
  /// Add a new fitness for the statistics.
  ///
  template<class T>
  void distribution<T>::add(T val)
  {
    if (!count)
      min = max = val;
    else if (val < min)
      min = val;
    else if (max < val)
      max = val;

    ++count;
    ++freq_[val];

    update_variance(val);
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
    const double c(1.0/std::log(2.0));

    double h(0.0);
    for (auto j(freq_.begin()); j != freq_.end(); ++j)
    {
      const double p(static_cast<double>(j->second) / count);
      h -= p * std::log(p) * c;
    }

    return h;
  }

  ///
  /// \param[in] val new fitness upon which statistics are recalculated.
  ///
  /// Calculate running variance and cumulative average fitness. The
  /// algorithm used is due to Knuth (Donald E. Knuth - The Art of Computer
  /// Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232.
  /// Addison-Wesley).
  /// This is an online algorithm
  /// (http://en.wikipedia.org/wiki/Online_algorithm).
  ///
  template<class T>
  void distribution<T>::update_variance(T val)
  {
    delta_ = val - mean;
    mean += delta_/count;

    // This expression uses the new value of mean.
    m2_ += delta_ * (val-mean);

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
  /// \return \c true if the object passes the internal consistency check.
  ///
  template<class T>
  bool distribution<T>::check() const
  {
    return
      (!std::isfinite(min) || !std::isfinite(mean) ||
       min <= mean+float_epsilon) &&
      (!std::isfinite(max) || !std::isfinite(mean) ||
       mean <= max+float_epsilon) &&
      (std::isnan(variance) || 0.0 <= variance+float_epsilon);
  }
}  // Namespace vita

#endif  // DISTRIBUTION_H
