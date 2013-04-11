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
    age_.clear();
    fit_.clear();
    length_.clear();

    functions_ = stats();
    terminals_ = stats();

    info_.clear();
  }

  ///
  /// \return a constant reference to the first statistical \a symbol we have
  ///         informations about.
  ///
  analyzer::const_iterator analyzer::begin() const
  {
    return info_.begin();
  }

  ///
  /// \return a constant reference (sentry) used for loops.
  ///
  analyzer::const_iterator analyzer::end() const
  {
    return info_.end();
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
  /// \return statistics about the age distribution in layer \a l.
  ///
  const distribution<unsigned> &analyzer::age_dist(size_t l)
  {
    return age_[l];
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
  unsigned analyzer::count(const individual &ind)
  {
    for (index_t i(0); i < ind.size(); ++i)
      for (category_t c(0); c < ind.env().sset.categories(); ++c)
      {
        const locus l{{i, c}};
        count(ind[l].sym.get(), false);
      }

    unsigned length(0);
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

    ++info_[sym].counter[active];

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
    age_[l].add(ind.age);

    length_.add(count(ind));

    if (f.isfinite())
      fit_.add(f);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool analyzer::debug() const
  {
    for (const auto &i : info_)
      if (i.second.counter[true] > i.second.counter[false])
        return false;

    for (const auto &l : age_)
      if (!l.second.debug())
        return false;

    if (!fit_.debug())
      return false;

    return length_.debug();
  }
}  // namespace vita
