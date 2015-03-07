/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014, 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INDIVIDUAL_H)
#define      VITA_INDIVIDUAL_H

#include <cmath>
#include <functional>
#include <iomanip>
#include <set>

#include "kernel/locus.h"
#include "kernel/symbol_set.h"
#include "kernel/ttable.h"
#include "kernel/vitafwd.h"

namespace vita
{
///
/// A single member of a `population`. Each individual contains a genome
/// which represents a possible solution to the task being tackled (i.e. a
/// point in the search space).
///
/// This class is the base class of every type of individual. The class:
/// * factorizes out common code;
/// * defines a minimum common interface.
///
/// \note AKA chromosome
///
class individual
{
public:
  individual(const environment &, const symbol_set &);

  /// This is a measure of how long an individual's family of genotypic
  /// material has been in the population. Randomly generated individuals,
  /// such as those that are created when the search algorithm are started,
  /// start with an age of 0. Each generation that an individual stays in the
  /// population (such as through elitism) its age is increased by one.
  /// Individuals that are created through mutation or recombination take the
  /// age of their oldest parent.
  /// This differs from conventional measures of age, in which individuals
  /// created through applying some type of variation to an existing
  /// individual (e.g. mutation or recombination) start with an age of 0.
  unsigned age() const { return age_; }
  void inc_age() { ++age_; }

  const environment &env() const { return *env_; }
  const symbol_set &sset() const { return *sset_; }

  // Visualization/output methods.
  virtual std::ostream &in_line(std::ostream &) const = 0;

protected:  // Protected data members
  // Note that syntactically distinct (but logically equivalent) individuals
  // have the same signature. This is a very interesting  property, useful
  // for individual comparison, information retrieval, entropy calculation...
  mutable hash_t signature_;

  unsigned age_;

  const environment *env_;
  const symbol_set *sset_;
};  // class individual

inline individual::individual(const environment &e, const symbol_set &ss)
  : signature_(), age_(0), env_(&e), sset_(&ss)
{
  assert(e.debug(true, true));
}

}  // namespace vita
#endif  // Include guard
