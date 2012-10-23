/**
 *
 *  \file ttable.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(TTABLE_H)
#define      TTABLE_H

#include <vector>

#include "kernel/evaluator.h"

namespace vita
{
  class individual;

  ///
  /// This is a 128bit stream used as individual signature / hash table
  /// look up key.
  ///
  struct hash_t
  {
    /// Hash signature is a 128 bit unsigned and is built by two 64 bit
    /// halves.
    hash_t(boost::uint64_t a = 0, boost::uint64_t b = 0) : p1(a), p2(b) {}

    /// Resets the content of hash_t.
    void clear() { p1 = 0; p2 = 0; }

    /// Standard equality operator for hash signature.
    bool operator==(hash_t h) const { return p1 == h.p1 && p2 == h.p2; }

    /// Standard inequality operator for hash signature.
    bool operator!=(hash_t h) const { return p1 != h.p1 || p2 != h.p2; }

    /// We assume that a string of 128 zero bits means empty.
    bool empty() const { return !p1 && !p2; }

    /// First half of the hash signature.
    boost::uint64_t p1;
    /// Second half of the hash signature.
    boost::uint64_t p2;
  };

  ///
  /// \param[out] o output stream.
  /// \param[in] h hash signature to be printed.
  ///
  /// Mainly useful for debugging / testing.
  ///
  inline
  std::ostream &operator<<(std::ostream &o, hash_t h)
  {
    return o << h.p1 << h.p2;
  }


  ///
  /// \a ttable \c class implements a hash table that links individuals to
  /// fitness (it's used by the \a evaluator_proxy \c class).
  /// The key used for table lookup is the individual's signature.
  /// During the evolution semantically equivalent (but syntactically distinct)
  /// individuals are often generated and \a ttable could give a significant
  /// speed improvement avoiding the recalculation of shared information.
  ///
  class ttable
  {
  public:
    explicit ttable(unsigned);
    ~ttable();

    void clear();
    void clear(const individual &);

    void insert(const individual &, const score_t &);

    bool find(const individual &, score_t *const) const;

    /// \return number of searches in the hash table
    /// Every call to the \c find method increment the counter.
    boost::uint64_t probes() const { return probes_; }

    /// \return number of successful searches in the hash table.
    boost::uint64_t hits() const { return hits_; }

    bool check() const;

  private:
    struct slot
    {
      hash_t       hash;
      score_t     score;
      unsigned birthday;
    };

    const boost::uint64_t k_mask;
    slot *const table_;

    unsigned period_;

    mutable boost::uint64_t probes_;
    mutable boost::uint64_t hits_;
  };

  /// \example example4.cc
  /// Performs a speed test on the transposition table (insert-find cycle).
}  // namespace vita

#endif  // TTABLE_H
