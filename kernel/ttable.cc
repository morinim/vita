/**
 *
 *  \file ttable.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "ttable.h"
#include "individual.h"

namespace vita
{
  ///
  /// \param[in] in input stream.
  /// \return \c true if hash_t loaded correctly.
  ///
  /// \note
  /// If the load operation isn't successful the current hash_t isn't changed.
  ///
  bool hash_t::load(std::istream &in)
  {
    hash_t tmp;

    if (!(in >> tmp.data[0] >> tmp.data[1]))
      return false;

    *this = tmp;

    return true;
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if hash_t was saved correctly.
  ///
  bool hash_t::save(std::ostream &out) const
  {
    out << data[0] << ' ' << data[1] << std::endl;

    return out.good();
  }

  ///
  /// \param[in] bits 2^\a bits is the number of elements of the table.
  ///
  /// Creates a new transposition (hash) table.
  ///
  ttable::ttable(unsigned bits)
    : k_mask((1 << bits) - 1), table_(new slot[1 << bits]), period_(1),
      probes_(0), hits_(0)
  {
    assert(debug());
  }

  ///
  /// Standard destructor.
  ///
  ttable::~ttable()
  {
    delete [] table_;
  }

  ///
  /// \param[in] u the signature of an individual.
  /// \return an index in the hash table.
  ///
  inline
  size_t ttable::index(const hash_t &h) const
  {
    return h.data[0] & k_mask;
  }

  ///
  /// Clears the content and the statistical informations of the table
  /// (allocated size isn't changed).
  ///
  void ttable::clear()
  {
    probes_ = 0;
    hits_   = 0;

    ++period_;

    //for (size_t i(0); i <= k_mask; ++i)
    //{
    //  table_[i].hash = hash_t();
    //  table_[i].fitness = {0.0, 0.0};
    //}
  }

  ///
  /// \param[in] ind individual whose informations we have to clear.
  ///
  /// Clears the cached information for individual \a ind.
  ///
  void ttable::clear(const individual &ind)
  {
    const hash_t h(ind.signature());

    table_[index(h)].hash = hash_t();
  }

  ///
  /// \param[in] ind the individual to look for.
  /// \param[out] fitness the fitness and the accuracy of the individual (if
  ///                     present).
  /// \return \c true if \a ind is found in the transposition table, \c false
  ///         otherwise.
  ///
  /// Looks for the fitness of an individual in the transposition table.
  ///
  bool ttable::find(const individual &ind, fitness_t *const fitness) const
  {
    ++probes_;

    const hash_t h(ind.signature());

    const slot &s(table_[index(h)]);

    const bool ret(period_ == s.birthday && h == s.hash);

    if (ret)
    {
      ++hits_;
      *fitness = s.fitness;
    }

    return ret;
  }

  ///
  /// \param[in] ind a (possibly) new individual to be stored in the table.
  /// \param[out] fitness the fitness and the accuracy of the individual.
  ///
  /// Stores fitness information in the transposition table.
  ///
  void ttable::insert(const individual &ind, const fitness_t &fitness)
  {
    slot s;
    s.hash     = ind.signature();
    s.fitness  =         fitness;
    s.birthday =         period_;

    table_[index(s.hash)] = s;
  }

  ///
  /// \return \c true if the table passes the internal consistency check.
  ///
  bool ttable::debug() const
  {
    return probes() >= hits();
  }
}  // namespace vita
