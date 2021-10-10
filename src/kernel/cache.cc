/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <mutex>

#include "kernel/cache.h"

namespace vita
{
///
/// Creates a new hash table.
///
/// \param[in] bits `2^bits` is the number of elements of the table
///
cache::cache(unsigned bits) : mutex_(), k_mask((1ull << bits) - 1),
                              table_(1ull << bits), seal_(1)
{
  Expects(bits);
  Ensures(is_valid());
}

///
/// \param[in] u the signature of an individual
/// \return      an index in the hash table
///
inline std::size_t cache::index(const hash_t &h) const
{
  return h.data[0] & k_mask;
}

///
/// Clears the content and the statistical informations of the table.
///
/// \note Allocated size isn't changed.
///
void cache::clear()
{
  std::unique_lock lock(mutex_);

  ++seal_;

  // for (auto &s : table_)
  // {
  //   s.hash = hash_t();
  //   s.fitness = {};
  // }
}

///
/// Clears the cached information for a specific individual.
///
/// \param[in] h individual's signature whose informations we have to clear
///
void cache::clear(const hash_t &h)
{
  std::unique_lock lock(mutex_);

  table_[index(h)].hash = hash_t();

  // An alternative to invalidate the slot:
  //   table_[index(h)].seal = 0;
  // It works because the first valid seal is 1.
}

///
/// Looks for the fitness of an individual in the transposition table.
///
/// \param[in] h individual's signature to look for
/// \return      the fitness of the individual. If the individuals isn't
///              present returns an empty fitness
///
const fitness_t &cache::find(const hash_t &h) const
{
  std::shared_lock lock(mutex_);

  const slot &s(table_[index(h)]);
  const bool ret(seal_ == s.seal && h == s.hash);

  if (ret)
    return s.fitness;

  static const fitness_t empty{};
  return empty;
}

///
/// Stores fitness information in the transposition table.
///
/// \param[in] h       a (possibly) new individual's signature to be stored in
///                    the table
/// \param[in] fitness the fitness of the individual
///
void cache::insert(const hash_t &h, const fitness_t &fitness)
{
  std::unique_lock lock(mutex_);

  slot s;
  s.hash    =       h;
  s.fitness = fitness;
  s.seal    =   seal_;

  table_[index(s.hash)] = s;
}

///
/// \param[in] in input stream
/// \return       `true` if the object is correctly loaded
///
/// \note
/// If the load operation isn't successful the current object isn't changed.
///
bool cache::load(std::istream &in)
{
  std::unique_lock lock(mutex_);

  decltype(seal_) t_seal;
  if (!(in >> t_seal))
    return false;

  std::size_t n;
  if (!(in >> n))
    return false;

  for (decltype(n) i(0); i < n; ++i)
  {
    slot s;
    s.seal = t_seal;

    if (!s.hash.load(in))
      return false;
    if (!s.fitness.load(in))
      return false;

    table_[index(s.hash)] = s;
  }

  seal_ = t_seal;

  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if the object was saved correctly
///
bool cache::save(std::ostream &out) const
{
  std::shared_lock lock(mutex_);

  out << seal_ << ' ' << '\n';

  std::size_t num(0);
  for (const auto &s : table_)
    if (!s.hash.empty())
      ++num;
  out << num << '\n';

  for (const auto &s : table_)
    if (s.seal == seal_ && !s.hash.empty())
    {
      s.hash.save(out);
      s.fitness.save(out);
    }

  return out.good();
}

///
/// \return `true` if the object passes the internal consistency check
///
bool cache::is_valid() const
{
  return true;
}

}  // namespace vita
