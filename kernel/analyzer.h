/**
 *
 *  \file analyzer.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(ANALYZER_H)
#define      ANALYZER_H

#include <list>
#include <map>
#include <set>

#include "distribution.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{
  struct stats
  {
    stats() : counter{0, 0} {}

    /// Typical use: \c counter[active] or \c counter[!active] (where \c active
    /// is a boolean).
    boost::uint64_t counter[2];
  };

  ///
  /// Analyzer takes a statistics snapshot of a set of individuals.
  /// Procedute:
  /// <ol>
  /// <li>
  ///   the set should be specified adding (analyzer::add method) one invividual
  ///   at time;
  /// </li>
  /// <li>
  ///   statistics can be checked executing the desidered methods.
  /// </li>
  /// </ol>
  /// Informations regard:
  /// \li the set as a whole (analyzer::fit_dist, analyzer::length dist,
  ///     analyzer::functions>, analyzer::terminals methods);
  /// \li symbols appearing in the set (accessed by analyzer::begin and
  ///     analyzer::end methods).
  ///
  class analyzer
  {
  public:
    // Type returned by \c begin() and \c end() methods to iterate through the
    // statistics of the various symbols.
    typedef std::map<const symbol *, stats>::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    analyzer();

    void add(const individual &, fitness_t);

    void clear();

    boost::uint64_t functions(bool) const;
    boost::uint64_t terminals(bool) const;

    const distribution<fitness_t> &fit_dist() const;
    const distribution<double> &length_dist() const;

    bool check() const;

  private:
    unsigned count(const individual &);
    void count(const symbol *const, bool);

    std::map<const symbol *, stats> info_;

    distribution<fitness_t> fit_;
    distribution<double> length_;

    stats functions_;
    stats terminals_;
  };
}  // namespace vita

#endif  // ANALYZER_H
