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
  /// \brief Simplifies the calculation of statistics regarding a sequence
  ///        (mean, variance, standard deviation, entropy, min and max)
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

  public:   // Serialization
    bool load(std::istream &);
    bool save(std::ostream &) const;

  public:  // Public data members
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

  template<> void distribution<fitness_t>::add(fitness_t);
  template<> bool distribution<fitness_t>::debug(bool) const;

#include "kernel/distribution_inl.h"
}  // namespace vita

#if defined(VITA_NO_LIB)
#  include "distribution.cc"
#endif

#endif  // Include guard
