/**
 *
 *  \file analyzer.cc
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *  
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along 
 *  with VITA. If not, see <http://www.gnu.org/licenses/>. 
 *
 */

#include "kernel/analyzer.h"

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

    functions_[0] = functions_[1] = 0;
    terminals_[0] = terminals_[1] = 0;

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
  boost::uint64_t analyzer::functions(bool eff) const
  {
    return functions_[eff];
  }

  ///
  /// \param[in] eff effective / noneffective terminals.
  /// \return number of terminals in the population.
  ///
  boost::uint64_t analyzer::terminals(bool eff) const
  {
    return terminals_[eff];
  }

  ///
  /// \return statistics about the fitness distribution of the individuals.
  ///
  const distribution<fitness_t> &analyzer::fit_dist() const
  {
    assert(_fit.check());

    return fit_;
  }

  ///
  /// \return statistic about the length distribution of the individuals.
  ///
  const distribution<double> &analyzer::length_dist() const
  {
    assert(length_.check());

    return length_;
  }

  ///
  /// \param[in] ind individual to be analyzed.
  /// \return effective length of individual we gathered statistics about.
  ///
  unsigned
  analyzer::count(const individual &ind)
  {
    for (unsigned i(0); i < ind.size(); ++i)
      count(ind[i].sym, false);

    unsigned length(0);
    for (individual::const_iterator it(ind); it(); ++it)
    {
      count(it->sym, true);
      ++length;
    }

    return length;
  }

  ///
  /// \param[in] sym symbol we are gathering statistics about.
  /// \param[in] active is this an active gene?
  ///
  /// Used by count(const individual &).
  ///
  void
  analyzer::count(const symbol *const sym, bool active)
  {
    ++info_[sym].counter[active];

    if (sym->terminal())
      ++terminals_[active];
    else
      ++functions_[active];
  }

  ///
  /// \param[in] ind new individual.
  /// \param[in] f fitness of the new individual.
  ///
  /// Adds a new individual to the pool used to calculate statistics.
  ///
  void
  analyzer::add(const individual &ind, fitness_t f)
  {
    length_.add(count(ind));

    if (!is_bad(f))
      fit_.add(f);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool
  analyzer::check() const
  {
    for (const_iterator i(begin()); i != end(); ++i)
      if (i->second.counter[true] > i->second.counter[false])
        return false;

    return fit_.check() && length_.check();
  }
}  // namespace vita
