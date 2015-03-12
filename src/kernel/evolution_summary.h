/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_SUMMARY_H)
#define      VITA_EVOLUTION_SUMMARY_H

#include "kernel/analyzer.h"

namespace vita
{
///
/// \brief A summary of evolution (results, statistics...)
///
/// \tparam T type of individual
///
template<class T>
class summary
{
public:  // Constructor and support functions
  summary();

  void clear();

public:   // Serialization
  bool load(std::istream &, const environment &, const symbol_set &);
  bool save(std::ostream &) const;

public:  // Public data members
  analyzer<T> az;

  struct
  {
    T         solution;
    fitness_t  fitness;
    double    accuracy;
  } best;

  /// Time (in milliseconds) elapsed from evolution beginning.
  double elapsed;

  std::uintmax_t crossovers, mutations;

  unsigned gen, last_imp;
};

#include "kernel/evolution_summary.tcc"

}  // namespace vita

#endif  // Include guard
