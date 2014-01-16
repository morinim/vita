/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(TTABLE_H)
#define      TTABLE_H

#include "kernel/environment.h"

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
    hash_t(std::uint64_t a = 0, std::uint64_t b = 0) : data{a, b} {}

    /// Resets the content of hash_t.
    void clear() { data[0] = 0; data[1] = 0; }

    /// Standard equality operator for hash signature.
    bool operator==(hash_t h) const
    { return data[0] == h.data[0] && data[1] == h.data[1]; }

    /// Standard inequality operator for hash signature.
    bool operator!=(hash_t h) const
    { return data[0] != h.data[0] || data[1] != h.data[1]; }

    /// Used to combine multiple hashes.
    hash_t operator^=(hash_t h)
    { data[0] ^= h.data[0]; data[1] ^= h.data[1]; return *this; }

    /// We assume that a string of 128 zero bits means empty.
    bool empty() const { return !data[0] && !data[1]; }

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  public:  // Public data members.
    std::uint_least64_t data[2];
  };



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
#if defined(CLONE_SCALING)
    void reset_seen();
#endif

    void insert(const individual &, const fitness_t &);

    bool find(const individual &, fitness_t *const) const;
    unsigned seen(const individual &) const;

    /// \return number of searches in the hash table
    /// Every call to the \c find method increment the counter.
    std::uintmax_t probes() const { return probes_; }

    /// \return number of successful searches in the hash table.
    std::uintmax_t hits() const { return hits_; }

    bool debug() const;

  public:   // Serialization.
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support methods.
    size_t index(const hash_t &) const;

  private:  // Private data members.
    struct slot
    {
      /// This is used as primary key for access to the table.
      hash_t       hash;
      /// The stored fitness of an individual.
      fitness_t fitness;
      /// Valid slots are recognized comparing their seal with the current one.
      unsigned     seal;
#if defined(CLONE_SCALING)
      /// How many times have we looked for this individual in the current run?
      mutable unsigned seen;
#endif
    };

    const std::uint64_t k_mask;
    slot *const table_;

    decltype(slot::seal) seal_;

    mutable std::uintmax_t probes_;
    mutable std::uintmax_t hits_;
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
    return o << h.data[0] << h.data[1];
  }

  /// \example example4.cc
  /// Performs a speed test on the transposition table (insert-find cycle).
}  // namespace vita

#endif  // TTABLE_H
