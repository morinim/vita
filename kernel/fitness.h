/**
 *
 *  \file fitness.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(FITNESS_H)
#define      FITNESS_H

#include <iostream>

#include "vita.h"

namespace vita
{
  ///
  /// \brief
  /// A value assigned to an individual which reflects how well the individual
  /// solves the task.
  ///
  class fitness_t
  {
  public:
    typedef double base_t;

    fitness_t() {}
    explicit fitness_t(size_t d) : vect(d) { assert(d); }
    fitness_t(std::initializer_list<base_t> l) : vect(l) {}

    /// Operation is performed by first comparing sizes and, if they match,
    /// the elements are compared sequentially using algorithm equal, which
    /// stops at the first mismatch.
    bool operator==(const fitness_t &f) const
    { return vect == f.vect; }

    /// Operation is performed by first comparing sizes and, if they match,
    /// the elements are compared sequentially using algorithm equal, which
    /// stops at the first mismatch.
    bool operator!=(const fitness_t &f) const
    { return vect != f.vect; }

    /// Behaves as if using algorithm lexicographical_compare, which compares
    /// the elements sequentially, stopping at the first mismatch.
    ///
    /// \note
    /// A lexicographical comparison is the kind of comparison generally used
    /// to sort words alphabetically in dictionaries; it involves comparing
    /// sequentially the elements that have the same position in both ranges
    /// against each other until one element is not equivalent to the other.
    /// The result of comparing these first non-matching elements is the result
    /// of the lexicographical comparison.
    /// If both sequences compare equal until one of them ends, the shorter
    /// sequence is lexicographically less than the longer one.
    bool operator>(const fitness_t &f) const
    {
      assert(vect.size() == f.vect.size());
      return vect > f.vect;

      // An alternative implementation:
      // > const size_t sup(vect.size());
      // > for (size_t i(0); i < sup; ++i)
      // >   if (vect[i] != f.vect[i])
      // >     return vect[i] > f.vect[i];
      // > return false;
    }

    /// Lexicographic ordering.
    /// \see fitness_t::operator>
    bool operator>=(const fitness_t &f) const
    { assert(vect.size() == f.vect.size()); return vect >= f.vect; }

    /// Lexicographic ordering.
    /// \see fitness_t::operator>
    bool operator<(const fitness_t &f) const
    { assert(vect.size() == f.vect.size()); return vect < f.vect; }

    /// Lexicographic ordering.
    /// \see fitness_t::operator>
    bool operator<=(const fitness_t &f) const
    { assert(vect.size() == f.vect.size()); return vect <= f.vect; }

    ///
    /// \param[in] f second term of comparison.
    /// \return \c true if \a this is a Pareto improvement of \a f.
    ///
    /// \a this dominates \a f (is a Pareto improvement) if each component of
    /// \a this is not strictly worst (less) than the correspondig component of
    /// \a f.
    bool dominating(const fitness_t &f) const
    {
      assert(vect.size() == f.vect.size());

      const size_t sup(vect.size());
      for (size_t i(0); i < sup; ++i)
        if (vect[i] < f.vect[i])
          return false;

      return true;
    }

    base_t operator[](size_t i) const
    { assert(i < vect.size()); return vect[i]; }

    base_t &operator[](size_t i)
    { assert(i < vect.size()); return vect[i]; }

    size_t size() const
    { return vect.size(); }

    bool empty() const
    { return vect.empty(); }

    ///
    /// \warning
    /// Do not change with a static const variable definition: danger of static
    /// initialization order fiasco.
    ///
    static fitness_t lowest(size_t s)
    {
      fitness_t f(s);
      for (size_t i(0); i < s; ++i)
        f[i] = std::numeric_limits<base_t>::lowest();

      return f;
    }

  private:
    std::vector<base_t> vect;
  };

  std::ostream &operator<<(std::ostream &, const fitness_t &);
}  // namespace vita

#endif  // FITNESS_H
