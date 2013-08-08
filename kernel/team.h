/**
 *
 *  \file team.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(TEAM_H)
#define      TEAM_H

#include "kernel/vita.h"

namespace vita
{
  ///
  /// \brief A collection of cooperating individuals used as a member of a
  ///        \a population.
  ///
  template<class T>
  class basic_team
  {
  public:
    basic_team(const environment &, const symbol_set &);

    // Visualization/output methods
    void graphviz(std::ostream &) const;
    void in_line(std::ostream &) const;
    void list(std::ostream &) const;
    void tree(std::ostream &) const;

    // Recombination operators.
    unsigned mutation();
    unsigned mutation(double);
    basic_team<T> crossover(const basic_team<T> &) const;

    typedef typename std::vector<T>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    const T &operator[](unsigned) const;

    unsigned individuals() const;
    unsigned size() const;
    unsigned eff_size() const;

    hash_t signature() const;

    bool operator==(const basic_team<T> &) const;
    unsigned distance(const basic_team<T> &) const;

    unsigned age() const;
    void inc_age();

    const environment &env() const;
    const symbol_set &sset() const;

    bool debug(bool = true) const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support methods.
    hash_t hash() const;

  private:  // Private data members.
    std::vector<T> individuals_;

    mutable hash_t signature_;
  };

  template<class T> std::ostream &operator<<(std::ostream &,
                                             const basic_team<T> &);

#include "team_inl.h"
}  // namespace vita

#endif  // TEAM_H
