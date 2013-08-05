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
  class team
  {
  public:
    team(const environment &, const symbol_set &);

    unsigned mutation();
    unsigned mutation(double);

    typedef typename std::vector<T>::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;
    const T &operator[](unsigned) const;

    unsigned size() const;
    unsigned eff_size() const;

    hash_t signature() const;

    bool operator==(const team<T> &) const;
    unsigned distance(const team<T> &) const;

    unsigned age() const;

    const environment &env() const;
    const symbol_set &sset() const;

    bool debug(bool = true) const;

  private:  // Private support methods.
    hash_t hash() const;

  private:  // Private data members.
    std::vector<T> individuals_;

    mutable hash_t signature_;
  };

  template<class T> std::ostream &operator<<(std::ostream &, const team<T> &);

#include "team_inl.h"
}  // namespace vita

#endif  // TEAM_H
