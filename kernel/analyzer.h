/**
 *
 *  \file analyzer.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
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

#include "kernel/distribution.h"
#include "kernel/individual.h"
#include "kernel/symbol.h"
#include "kernel/team.h"

namespace vita
{
  ///
  /// \brief Analyzer takes a statistics snapshot of a population
  ///
  /// \tparam T type of the elements of the population (individuals).
  ///
  /// Procedure:
  /// 1. the population set should be loaded adding (basic_analyzer::add
  ///    method) one invividual at time;
  /// 2. statistics can be checked executing the desidered methods.
  ///
  /// Informations regard:
  /// * the set as a whole (analyzer::fit_dist, analyzer::length dist,
  ///   analyzer::functions, analyzer::terminals methods);
  /// * symbols appearing in the set (accessed via basic_analyzer::begin and
  ///   basic_analyzer::end methods).
  ///
  template<class T>
  class basic_analyzer
  {
  public:
    struct sym_counter
    {
      sym_counter() : counter{0, 0} {}

      /// Typical use: \c counter[active] or \c counter[!active] (where
      /// \c active is a boolean).
      std::uintmax_t counter[2];
    };

    /// Type returned by \c begin() and \c end() methods to iterate through the
    /// statistics of the various symbols.
    typedef typename std::map<const symbol *, sym_counter>::const_iterator
      const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    basic_analyzer();

    void add(const T &, const fitness_t &, unsigned);

    void clear();

    std::uintmax_t functions(bool) const;
    std::uintmax_t terminals(bool) const;

    const distribution<double> &age_dist() const;
    const distribution<fitness_t> &fit_dist() const;
    const distribution<double> &length_dist() const;

    const distribution<double> &age_dist(unsigned) const;
    const distribution<fitness_t> &fit_dist(unsigned) const;

    bool debug() const;

  private:  // Private support methods.
    unsigned count(const T &);
    void count(const symbol *const, bool);

  private:  // Private data members.
    struct layer_stat
    {
      distribution<double> age;
      distribution<fitness_t> fitness;
    };

    std::map<const symbol *, sym_counter> sym_counter_;
    std::map<unsigned, layer_stat> layer_stat_;

    distribution<fitness_t> fit_;
    distribution<double> age_;
    distribution<double> length_;

    sym_counter functions_;
    sym_counter terminals_;
  };  // class basic_analyzer

  using analyzer = basic_analyzer<individual>;

#include "analyzer_inl.h"
}  // namespace vita

#endif  // ANALYZER_H
