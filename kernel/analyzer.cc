/**
 *
 *  \file analyzer.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "analyzer.h"

namespace vita
{
  ///
  /// New empty analyzer.
  ///
  analyzer::analyzer()
  {
    clear();
  }

  ///
  /// Resets gathered statics.
  ///
  void analyzer::clear()
  {
    layer_info_.clear();
    fit_.clear();
    length_.clear();

    functions_ = stat_sym_counter();
    terminals_ = stat_sym_counter();

    sym_counter_.clear();
  }

  ///
  /// \return a constant reference to the first statistical \a symbol we have
  ///         informations about.
  ///
  analyzer::const_iterator analyzer::begin() const
  {
    return sym_counter_.begin();
  }

  ///
  /// \return a constant reference (sentry) used for loops.
  ///
  analyzer::const_iterator analyzer::end() const
  {
    return sym_counter_.end();
  }

  ///
  /// \param[in] eff effective / noneffective functions.
  /// \return number of functions in the population.
  ///
  std::uintmax_t analyzer::functions(bool eff) const
  {
    return functions_.counter[eff];
  }

  ///
  /// \param[in] eff effective / noneffective terminals.
  /// \return number of terminals in the population.
  ///
  std::uintmax_t analyzer::terminals(bool eff) const
  {
    return terminals_.counter[eff];
  }

  ///
  /// \param[in] l a layer of the population.
  /// \return statistics about the age and fitness distribution in layer \a l.
  ///
  const stat_layer &analyzer::layer_info(size_t l) const
  {
    return layer_info_.at(l);
  }

  ///
  /// \return statistics about the fitness distribution of the individuals.
  ///
  const distribution<fitness_t> &analyzer::fit_dist() const
  {
    assert(fit_.debug());

    return fit_;
  }

  ///
  /// \return statistic about the length distribution of the individuals.
  ///
  const distribution<double> &analyzer::length_dist() const
  {
    assert(length_.debug());

    return length_;
  }

  ///
  /// \param[in] ind individual to be analyzed.
  /// \return effective length of individual we gathered statistics about.
  ///
  size_t analyzer::count(const individual &ind)
  {
    for (index_t i(0); i < ind.size(); ++i)
      for (category_t c(0); c < ind.env().sset.categories(); ++c)
      {
        const locus l{{i, c}};
        count(ind[l].sym.get(), false);
      }

    size_t length(0);
    for (individual::const_iterator it(ind); it(); ++it)
    {
      count(it->sym.get(), true);
      ++length;
    }

    return length;
  }

  ///
  /// \param[in] sym symbol we are gathering statistics about.
  /// \param[in] active is this an active gene?
  ///
  /// Used by \c count(const individual &)
  ///
  void analyzer::count(const symbol *const sym, bool active)
  {
    assert(sym);

    ++sym_counter_[sym].counter[active];

    if (sym->terminal())
      ++terminals_.counter[active];
    else
      ++functions_.counter[active];
  }

  ///
  /// \param[in] ind new individual.
  /// \param[in] f fitness of the new individual.
  /// \param[in] l the layer of the population the individual is placed in.
  ///
  /// Adds a new individual to the pool used to calculate statistics.
  ///
  void analyzer::add(const individual &ind, const fitness_t &f, size_t l)
  {
    layer_info_[l].age.add(ind.age);
    layer_info_[l].fitness.add(f);

    length_.add(count(ind));

    if (f.isfinite())
      fit_.add(f);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool analyzer::debug() const
  {
    for (const auto &i : sym_counter_)
      if (i.second.counter[true] > i.second.counter[false])
        return false;

    for (const auto &l : layer_info_)
    {
      if (!l.second.age.debug())
        return false;

      if (!l.second.fitness.debug())
        return false;
    }

    if (!fit_.debug())
      return false;

    return length_.debug();
  }
}  // namespace vita
