/**
 *
 *  \file ttable.h
 *
 *  \author Manlio Morini
 *  \date 2011/01/09
 *
 *  This file is part of VITA
 *
 */

#if !defined(TTABLE_H)
#define      TTABLE_H

#include "fitness.h"

#include <vector>

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
    //typedef boost::uint64_t hash_t;
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

    boost::uint64_t probes() const { return _probes; }
    boost::uint64_t hits() const { return _hits; }

    bool check() const;

  private:
    static hash_t hash(const std::vector<boost::uint8_t> &);

    struct slot
    {
      hash_t   hash;
      fitness_t fit;
    };

    const boost::uint64_t mask;
    slot *const table;

    mutable boost::uint64_t _probes;
    mutable boost::uint64_t _hits;
  };

  /// \example example4.cc
  /// Performs a speed test on the transposition table (insert-find cycle).
}  // namespace vita

#endif  // TTABLE_H
