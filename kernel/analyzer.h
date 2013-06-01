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

#include "distribution.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{
  ///
  /// \brief Analyzer takes a statistics snapshot of a set of individuals
  ///
  /// Procedure:
  /// 1. the the set should be loaded adding (analyzer::add method) one
  ///    invividual at time;
  /// 2. statistics can be checked executing the desidered methods.
  ///
  /// Informations regard:
  /// * the set as a whole (analyzer::fit_dist, analyzer::length dist,
  ///   analyzer::functions, analyzer::terminals methods);
  /// * symbols appearing in the set (accessed by analyzer::begin and
  ///   analyzer::end methods).
  ///
  class analyzer
  {
  public:
    struct stat_sym_counter
    {
      stat_sym_counter() : counter{0, 0} {}

      /// Typical use: \c counter[active] or \c counter[!active] (where
      /// \c active is a boolean).
      std::uintmax_t counter[2];
    };

    /// Type returned by \c begin() and \c end() methods to iterate through the
    /// statistics of the various symbols.
    typedef std::map<const symbol *, stat_sym_counter>::const_iterator
    const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    analyzer();

    void add(const individual &, const fitness_t &);

    void clear();

    std::uintmax_t functions(bool) const;
    std::uintmax_t terminals(bool) const;

    const distribution<double> &age_dist() const;
    const distribution<fitness_t> &fit_dist() const;
    const distribution<double> &length_dist() const;

    bool debug() const;

  private:  // Private support methods.
    size_t count(const individual &);
    void count(const symbol *const, bool);

  private:  // Private data members.
    std::map<const symbol *, stat_sym_counter> sym_counter_;

    distribution<fitness_t> fit_;
    distribution<double> age_;
    distribution<double> length_;

    stat_sym_counter functions_;
    stat_sym_counter terminals_;
  };  // class analyzer

  struct stat_layer
  {
    distribution<double> age;
    distribution<fitness_t> fitness;
  };
}  // namespace vita

#endif  // ANALYZER_H
