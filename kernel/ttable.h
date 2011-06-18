/**
 *
 *  \file ttable.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
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

#if !defined(TTABLE_H)
#define      TTABLE_H

#include <vector>

#include "kernel/fitness.h"

namespace vita
{
  class individual;

  ///
  /// \a ttable \c class implements a hash table that links individuals to their
  /// fitness (it's used by the \a evaluator_proxy \c class).
  /// Note: \a ttable exploits a byte level representation of an individual
  /// obtained from the \c individual::pack function. This function should
  /// map sintatically distinct (but logically equivalent) individuals to the
  /// same byte stream. During the evolution semantically equivalent individuals
  /// are often generated and \a ttable could give a significant speed
  /// improvement.
  ///
  class ttable
  {
  public:
    // typedef boost::uint64_t hash_t;
    struct hash_t
    {
      hash_t(boost::uint64_t a = 0, boost::uint64_t b = 0) : p1(a), p2(b) {}
      bool operator==(hash_t h) const { return p1 == h.p1 && p2 == h.p2; }

      boost::uint64_t p1;
      boost::uint64_t p2;
    };

    explicit ttable(unsigned);
    ~ttable();

    void clear();

    void insert(const individual &, fitness_t);

    bool find(const individual &, fitness_t *const) const;

    boost::uint64_t probes() const { return probes_; }
    boost::uint64_t hits() const { return hits_; }

    bool check() const;

  private:
    static hash_t hash(const std::vector<boost::uint8_t> &);

    struct slot
    {
      hash_t   hash;
      fitness_t fit;
    };

    const boost::uint64_t k_mask;
    slot *const table_;

    mutable boost::uint64_t probes_;
    mutable boost::uint64_t hits_;
  };

  /// \example example4.cc
  /// Performs a speed test on the transposition table (insert-find cycle).
}  // namespace vita

#endif  // TTABLE_H
