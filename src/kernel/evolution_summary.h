/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_SUMMARY_H)
#define      VITA_EVOLUTION_SUMMARY_H

#include <chrono>

#include "kernel/analyzer.h"
#include "kernel/model_measurements.h"

namespace vita
{
///
/// A summary of evolution (results, statistics...).
///
/// \tparam T type of individual
///
template<class T>
class summary
{
public:
  // --- Constructor and support functions ---
  summary();

  void clear();

  // --- Serialization ---
  bool load(std::istream &, const problem &);
  bool save(std::ostream &) const;

  // --- Public data members ---
  analyzer<T> az;

  struct
  {
    T               solution;
    model_measurements score;
  } best;

  /// Time elapsed from evolution beginning.
  std::chrono::milliseconds elapsed;

  /// Number of crossovers performed.
  std::uintmax_t crossovers;

  /// Number of mutations performed.
  std::uintmax_t mutations;

  unsigned gen, last_imp;
};

#include "kernel/evolution_summary.tcc"

}  // namespace vita

#endif  // include guard
