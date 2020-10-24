/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DISTRIBUTION_H)
#define      VITA_DISTRIBUTION_H

#include <cmath>
#include <iomanip>
#include <map>

#include "kernel/log.h"
#include "utility/utility.h"

namespace vita
{
///
/// Simplifies the calculation of statistics regarding a sequence (mean,
/// variance, standard deviation, entropy, min and max).
///
template<class T>
class distribution
{
public:
  distribution();

  void clear();

  template<class U> void add(U);

  std::uintmax_t count() const;
  double entropy() const;
  T max() const;
  T mean() const;
  T min() const;
  const std::map<T, std::uintmax_t> &seen() const;
  T standard_deviation() const;
  T variance() const;

  bool is_valid() const;

public:   // Serialization
  bool load(std::istream &);
  bool save(std::ostream &) const;

private:  // Private methods
  void update_variance(T);

private:  // Private data members
  std::map<T, std::uintmax_t> seen_;

  T m2_;
  T max_;
  T mean_;
  T min_;

  std::uintmax_t count_;
};

#include "kernel/distribution.tcc"
}  // namespace vita

#endif  // Include guard
