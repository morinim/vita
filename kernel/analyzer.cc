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
    length_.clear();

    fit_.clear();

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
  ///
  /// Adds a new individual to the pool used to calculate statistics.
  ///
  void analyzer::add(const individual &ind, fitness_t f)
  {
    length_.add(count(ind));

    if (std::isfinite(f))
      fit_.add(f);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool analyzer::debug() const
  {
    for (const_iterator i(begin()); i != end(); ++i)
      if (i->second.counter[true] > i->second.counter[false])
        return false;

    return fit_.debug() && length_.debug();
  }
}  // namespace vita
