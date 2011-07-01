/**
 *
 *  \file analyzer.h
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

#if !defined(ANALYZER_H)
#define      ANALYZER_H

#include <list>
#include <map>
#include <set>

#include "kernel/vita.h"
#include "kernel/distribution.h"
#include "kernel/individual.h"
#include "kernel/symbol.h"

namespace vita
{
  struct stats
  {
    unsigned counter[2];
  };

  ///
  /// Analyzer takes a statistics snapshot of a set of individuals. First
  /// the set should be specified adding (\a add method) one \a invividual
  /// at time, then statistics are calculated executing the \a run method.
  /// Informations regard:
  /// * the set as a whole (\a fit_dist, \a length dist, \a functions,
  ///   \a terminals methods);
  /// * symbols appearing in the set (accessed by \a begin and \a end methods).
  ///
  class analyzer
  {
  public:
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

    boost::uint64_t functions_[2];
    boost::uint64_t terminals_[2];
  };
}  // namespace vita

#endif  // ANALYZER_H
