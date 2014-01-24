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

#if !defined(VITA_ANALYZER_H)
#define      VITA_ANALYZER_H

#include <map>

#include "kernel/distribution.h"
#include "kernel/symbol.h"

namespace vita
{
  template<class T>
  class core_analyzer
  {
  public:
    struct sym_counter
    {
      sym_counter() : counter{0, 0} {}

      /// Typical use: \c counter[active] or \c counter[!active] (where
      /// \c active is a boolean).
      std::uintmax_t counter[2];
    };

    /// Type returned by begin() and end() methods to iterate through the
    /// statistics of the various symbols.
    typedef typename std::map<const symbol *, sym_counter>::const_iterator
      const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    core_analyzer();

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

  protected:  // Protected support methods
    virtual unsigned count(const T &) = 0;  // <-- VIRTUAL
    void count(const symbol *const, bool);

  private:  // Private data members
    /// This comparator is useful for debugging purpose: when we insert a
    /// symbol pointer in an ordered container, it induces a well defined
    /// order. Without this the default comparison for pointers has a
    /// unspecified (and not necessarily stable & consistent) behaviour.
    /// Well defined order means a simple way of debug statistics.
    struct cmp_symbol_ptr
    {
      bool operator()(const symbol *a, const symbol *b) const
      { return a->opcode() < b->opcode(); }
    };
    std::map<const symbol *, sym_counter, cmp_symbol_ptr> sym_counter_;

    struct layer_stat
    {
      distribution<double> age;
      distribution<fitness_t> fitness;
    };
    std::map<unsigned, layer_stat> layer_stat_;

    distribution<fitness_t> fit_;
    distribution<double> age_;
    distribution<double> length_;

    sym_counter functions_;
    sym_counter terminals_;
  };  // core_analyzer

  ///
  /// \brief Analyzer takes a statistics snapshot of a population
  ///
  /// \tparam T type of individual
  ///
  /// Procedure:
  /// 1. the population set should be loaded adding (analyzer::add method) one
  ///    individual at time;
  /// 2. statistics can be checked executing the desidered methods.
  ///
  /// Informations regard:
  /// * the set as a whole (analyzer::fit_dist, analyzer::length_dist,
  ///   analyzer::functions, analyzer::terminals methods);
  /// * symbols appearing in the set (accessed via analyzer::begin and
  ///   analyzer::end methods).
  ///
  template<class T>
  class analyzer : public core_analyzer<T>
  {
  public:
    using analyzer::core_analyzer::core_analyzer;

  private:  // Private support methods
    virtual unsigned count(const T &) override;
  };

  ///
  /// \brief Analyzer specialization for populations of teams.
  ///
  /// \tparam T type of individual
  ///
  template<class T>
  class analyzer<team<T>> : public core_analyzer<team<T>>
  {
  public:
    using analyzer::core_analyzer::core_analyzer;

  private:  // Private support methods
    virtual unsigned count(const team<T> &) override;
  };

#include "kernel/analyzer_inl.h"
}  // namespace vita

#endif  // include guard
