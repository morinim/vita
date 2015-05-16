/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_TEAM_H)
#define      VITA_TEAM_H

#include "kernel/environment.h"
#include "kernel/ttable.h"

namespace vita
{
///
/// \brief A collection of cooperating individuals used as a member of
///        vita::population
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
///   an object of evolution.
///
/// \note
/// The team size has to be large enough to cause an improved prediction
/// compared to the traditional approach, i.e. team size one (but the
/// complexity of the search space and the training time, respectively,
/// grow exponentially with the number of coevolved programs).
///
/// \see
/// "Evolving Teams of Predictors with Linear Genetic Programming"
/// (Markus Braimer, Wolfgang Banzhaf).
///
template<class T>
class team
{
public:
  explicit team(const environment &);
  explicit team(std::vector<T>);

  // Visualization/output methods
  void graphviz(std::ostream &) const;
  std::ostream &in_line(std::ostream &) const;
  std::ostream &list(std::ostream &) const;
  std::ostream &tree(std::ostream &) const;

  // Recombination operators.
  unsigned mutation();
  unsigned mutation(double);
  team<T> crossover(team<T>) const;

  const T &operator[](unsigned) const;

  unsigned individuals() const;
  unsigned eff_size() const;

  hash_t signature() const;

  unsigned age() const;
  void inc_age();

  const environment &env() const;

  bool debug(bool = true) const;

private:
  using members_t = std::vector<T>;

public:  // Iterators
  using const_iterator = typename members_t::const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

public:   // Serialization
  bool load(std::istream &);
  bool save(std::ostream &) const;

private:  // Private support methods
  hash_t hash() const;

private:  // Private data members
  members_t individuals_;

  mutable hash_t signature_;
};

// ***********************************************************************
// *  Comparison operators                                               *
// ***********************************************************************
template<class T> bool operator==(const team<T> &, const team<T> &);
template<class T> unsigned distance(const team<T> &, const team<T> &);

template<class T> std::ostream &operator<<(std::ostream &, const team<T> &);

// The SFINAE way of recognize a team.
template<class T> struct is_team : std::false_type {};
template<class T> struct is_team<team<T>> : std::true_type {};

template<class T> struct not_team : std::true_type {};
template<class T> struct not_team<team<T>> : std::false_type {};

#include "kernel/team.tcc"

}  // namespace vita

#endif  // Include guard
