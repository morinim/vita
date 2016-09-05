/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CACHE_H)
#define      VITA_CACHE_H

#include "kernel/environment.h"

namespace vita
{
///
/// This is a 128bit stream used as individual signature / hash table
/// look up key.
///
struct hash_t
{
  /// Hash signature is a 128 bit unsigned and is built by two 64 bit
  /// halves.
  explicit hash_t(std::uint64_t a = 0, std::uint64_t b = 0) : data{a, b} {}

  /// Resets the content of hash_t.
  void clear() { data[0] = data[1] = 0; }

  /// Standard equality operator for hash signature.
  bool operator==(hash_t h) const
  { return data[0] == h.data[0] && data[1] == h.data[1]; }

  /// Standard inequality operator for hash signature.
  bool operator!=(hash_t h) const
  { return data[0] != h.data[0] || data[1] != h.data[1]; }

  /// \brief Used to combine multiple hashes.
  ///
  /// \note
  /// In spite of its handy bit-mixing properties, XOR is not a good way to
  /// combine hashes due to its commutativity (e.g. see
  /// http://stackoverflow.com/q/5889238/3235496).
  void combine(hash_t h)
  {
    // This combiner is a tip from Bob Jenkins. An alternative from Boost is:
    // data[i] ^= h.data[i] + 0x9e3779b9 + (data[i] << 6) + (dati[i] >> 2);
    data[0] += 11 * h.data[0];
    data[1] += 13 * h.data[1];
  }

  /// We assume that a string of 128 zero bits means empty.
  bool empty() const { return !data[0] && !data[1]; }

  // Serialization.
  bool load(std::istream &);
  bool save(std::ostream &) const;

  // Data members.
  std::uint_least64_t data[2];
};

std::ostream &operator<<(std::ostream &, hash_t);

///
/// \brief Implements a hash table that links individuals' signature to
///        fitness (mainly used by the evaluator_proxy class).
///
/// During the evolution semantically equivalent (but syntactically distinct)
/// individuals are often generated and cache can give a significant speed
/// improvement avoiding the recalculation of shared information.
///
class cache
{
public:
  explicit cache(std::uint8_t);

  void clear();
  void clear(const hash_t &);
#if defined(CLONE_SCALING)
  void reset_seen();
#endif

  void insert(const hash_t &, const fitness_t &);

  const fitness_t &find(const hash_t &) const;

  unsigned seen(const hash_t &) const;

  /// \return number of searches in the hash table.
  /// Every call to the find method increment the counter.
  std::uintmax_t probes() const { return probes_; }

  /// \return number of successful searches in the hash table.
  std::uintmax_t hits() const { return hits_; }

  bool debug() const;

  // Class has pointer data members so disabling the copy constructor /
  // `operator=()` is a good idea (see "Effective C++").
  DISALLOW_COPY_AND_ASSIGN(cache);

  // Serialization.
  bool load(std::istream &);
  bool save(std::ostream &) const;

private:
  // Private support methods.
  std::size_t index(const hash_t &) const;

  // Private data members.
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
  std::vector<slot>   table_;

  decltype(slot::seal) seal_;

  mutable std::uintmax_t probes_;
  mutable std::uintmax_t   hits_;
};

/// \example example4.cc
/// Performs a speed test on the transposition table (insert-find cycle).
}  // namespace vita

#endif  // include guard
