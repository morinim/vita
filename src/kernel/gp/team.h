/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_TEAM_H)
#define      VITA_TEAM_H

#include <algorithm>

#include "kernel/cache.h"
#include "kernel/individual.h"

namespace vita
{
///
/// A collection of cooperating individuals used as a member of
/// vita::population.
///
/// \tparam T type of the elements of the team (individuals)
///
/// In generals teams of individuals can be implemented in different ways.
/// * Firstly, a certain number of individuals can be selected randomly from
///   the population and evaluated in combination as a team (but we have a
///   credit assignment problem).
/// * Secondly, team members can be evolved in separate subpopulations which
///   provide a more specialized development.
/// * We can use an explicit team representation that is considered as one
///   individual by the evolutionary algorithm. The population is subdivided
///   into fixed, equal-sized groups of individuals. Each program is
///   assigned a fixed position index in its team (program vector). The
///   members of a team undergo a coevolutionary process because they are
///   always selected, evaluated and varied simultaneously. This eliminates
///   the credit assignment problem and renders the composition of teams an
///   object of evolution.
///
/// \note
/// The team size has to be large enough to cause an improved prediction
/// compared to the traditional approach, i.e. team size one (but the
/// complexity of the search space and the training time, respectively,
/// grow exponentially with the number of coevolved programs).
///
/// \see https://github.com/morinim/vita/wiki/bibliography#16
///
template<class T>
class team
{
public:
  team();
  explicit team(unsigned);
  explicit team(const problem &);
  explicit team(std::vector<T>);

  // Recombination operators.
  unsigned mutation(double, const problem &);

  const T &operator[](unsigned) const;

  bool empty() const;
  unsigned individuals() const;
  unsigned active_symbols() const;

  hash_t signature() const;

  unsigned age() const;
  void inc_age();

  bool is_valid() const;

  // Iterators.
  using members_t = std::vector<T>;
  using const_iterator = typename members_t::const_iterator;
  using value_type = typename members_t::value_type;
  const_iterator begin() const;
  const_iterator end() const;

  // Serialization.
  bool load(std::istream &, const symbol_set &);
  bool save(std::ostream &) const;

  template<class U> friend team<U> crossover(const team<U> &, const team<U> &);

private:
  // Private support methods.
  hash_t hash() const;

  // Private data members.
  members_t individuals_;

  mutable hash_t signature_;
};

// ***********************************************************************
// *  Comparison operators                                               *
// ***********************************************************************
template<class T> bool operator==(const team<T> &, const team<T> &);
template<class T> bool operator!=(const team<T> &, const team<T> &);
template<class T> unsigned distance(const team<T> &, const team<T> &);

template<class T> team<T> crossover(const team<T> &, const team<T> &);

template<class T> std::ostream &operator<<(std::ostream &, const team<T> &);

// The SFINAE way of recognizing a team.
template<class T> struct is_team : std::false_type
{ enum {value = false}; };
template<class T> struct is_team<team<T>> : std::true_type
{ enum {value = true}; };

template<class T> struct not_team : std::true_type
{ enum {value = true}; };
template<class T> struct not_team<team<T>> : std::false_type
{ enum {value = false}; };

#include "kernel/gp/team.tcc"

}  // namespace vita

#endif  // include guard
