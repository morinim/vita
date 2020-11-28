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

#if !defined(VITA_ANALYZER_H)
#define      VITA_ANALYZER_H

#include <map>

#include "kernel/distribution.h"
#include "kernel/ga/i_de.h"
#include "kernel/ga/i_ga.h"
#include "kernel/gp/symbol.h"

namespace vita
{

///
/// Analyzer takes a statistics snapshot of a population.
///
/// \tparam T type of individual
///
/// Procedure:
/// 1. the population set should be loaded adding one individual at time
///    (analyzer::add method);
/// 2. statistics can be checked calling specific methods.
///
/// You can get information about:
/// - the set as a whole (`age_dist()`, `fit_dist()`, `length_dist()`,
///   `functions()`, `terminals()` methods);
/// - symbols appearing in the set (accessed via `begin()` / `end()` methods);
/// - grouped information (`age_dist(unsigned)`, `fit_dist(unsigned)`).
///
template<class T>
class analyzer
{
public:
  struct sym_counter
  {
    /// Typical use: `counter[active]` or `counter[!active]` (where
    /// `active` is a boolean).
    std::uintmax_t counter[2] = {0, 0};
  };

  analyzer();

  void add(const T &, const fitness_t &, unsigned = 0);

  void clear();

  std::uintmax_t functions(bool) const;
  std::uintmax_t terminals(bool) const;

  const distribution<double> &age_dist() const;
  const distribution<fitness_t> &fit_dist() const;
  const distribution<double> &length_dist() const;

  const distribution<double> &age_dist(unsigned) const;
  const distribution<fitness_t> &fit_dist(unsigned) const;

  /// Type returned by begin() and end() methods to iterate through the
  /// statistics of the various symbols.
  using const_iterator =
    typename std::map<const symbol *, sym_counter>::const_iterator;

  const_iterator begin() const;
  const_iterator end() const;

  bool is_valid() const;

private:
  void count(const symbol *, bool);
  std::size_t count(const T &);
  std::size_t count_team(const T &, std::true_type);
  template<class U> std::size_t count_team(const U &, std::false_type);
  template<class U> std::size_t count_introns(const U &, std::true_type);
  template<class U> std::size_t count_introns(const U &, std::false_type);

  // This comparator is useful for debugging purpose: when we insert a
  // symbol pointer in an ordered container, it induces a well defined
  // order. Without this the default comparison for pointers has a
  // unspecified (and not necessarily stable & consistent) behaviour.
  // Well defined order means a simple way of debugging statistics.
  struct cmp_symbol_ptr
  {
    bool operator()(const symbol *a, const symbol *b) const
    { return a->opcode() < b->opcode(); }
  };
  std::map<const symbol *, sym_counter, cmp_symbol_ptr> sym_counter_;

  struct group_stat
  {
    distribution<double>        age;
    distribution<fitness_t> fitness;
  };
  std::map<unsigned, group_stat> group_stat_;

  distribution<fitness_t> fit_;
  distribution<double>    age_;
  distribution<double> length_;

  sym_counter functions_;
  sym_counter terminals_;
};  // analyzer

#include "kernel/analyzer.tcc"
}  // namespace vita

#endif  // include guard
