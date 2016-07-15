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

#include "kernel/cache.h"

namespace vita
{
///
/// \param[in] in input stream.
/// \return `true` if hash_t loaded correctly.
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
/// \return `true` if hash_t was saved correctly.
///
bool hash_t::save(std::ostream &out) const
{
  out << data[0] << ' ' << data[1] << '\n';

  return out.good();
}

///
/// \param[in] bits `2^bits` is the number of elements of the table.
///
/// Creates a new hash table.
///
cache::cache(std::uint8_t bits)
  : k_mask((1u << bits) - 1), table_(1u << bits),
    seal_(1), probes_(0), hits_(0)
{
  Expects(bits);
  Ensures(debug());
}

///
/// \param[in] u the signature of an individual.
/// \return an index in the hash table.
///
inline std::size_t cache::index(const hash_t &h) const
{
  return h.data[0] & k_mask;
}

///
/// Clears the content and the statistical informations of the table
/// (allocated size isn't changed).
///
void cache::clear()
{
  probes_ = hits_ = 0;

  ++seal_;

  // for (auto &s : table_)
  // {
  //   s.hash = hash_t();
  //   s.fitness = {};
  // }
}

///
/// \param[in] h individual's signature whose informations we have to clear.
///
/// Clears the cached information for individual `h`.
///
void cache::clear(const hash_t &h)
{
  table_[index(h)].hash = hash_t();

  // An alternative to invalidate the slot:
  //   table_[index(h)].seal = 0;
  // It works because the first valid seal is 1.
}

///
/// \brief Resets the `seen` counter
///
#if defined(CLONE_SCALING)
void cache::reset_seen()
{
  probes_ = hits_ = 0;

  for (auto &s : table_)
    s.seen = 0;
}
#endif

///
/// \brief Looks for the fitness of an individual in the transposition table
///
/// \param[in] h individual's signature to look for.
/// \param[out] fitness the fitness of the individual (if present).
/// \return `true` if `h` is found in the transposition table, `false`
///         otherwise.
///
bool cache::find(const hash_t &h, fitness_t *const fitness) const
{
  ++probes_;

  const slot &s(table_[index(h)]);

  const bool ret(seal_ == s.seal && h == s.hash);

  if (ret)
  {
#if defined(CLONE_SCALING)
    ++s.seen;
#endif
    ++hits_;
    *fitness = s.fitness;
  }

  return ret;
}

///
/// \param[in] h individual's signature to look for.
/// \return number of times `h` has been looked for.
///
unsigned cache::seen(const hash_t &h) const
{
  const slot &s(table_[index(h)]);

  const bool ret(seal_ == s.seal && h == s.hash);

#if defined(CLONE_SCALING)
  return ret ? s.seen : 0;
#else
  return ret;
#endif
}

///
/// \param[in] h a (possibly) new individual's signature to be stored in the
///              table.
/// \param[in] fitness the fitness of the individual.
///
/// Stores fitness information in the transposition table.
///
void cache::insert(const hash_t &h, const fitness_t &fitness)
{
  slot s;
  s.hash    =       h;
  s.fitness = fitness;
  s.seal    =   seal_;
#if defined(CLONE_SCALING)
  s.seen    =       1;
#endif

  table_[index(s.hash)] = s;
}

///
/// \param[in] in input stream.
/// \return `true` if the object loaded correctly.
///
/// \note
/// If the load operation isn't successful the current object isn't changed.
///
bool cache::load(std::istream &in)
{
  decltype(seal_) t_seal;
  if (!(in >> t_seal))
    return false;

  decltype(probes_) t_probes;
  if (!(in >> t_probes))
    return false;

  decltype(hits_) t_hits;
  if (!(in >> t_hits))
    return false;

  std::size_t n;
  if (!(in >> n))
    return false;

  for (decltype(n) i(0); i < n; ++i)
  {
    slot s;

    if (!s.hash.load(in))
      return false;
    if (!s.fitness.load(in))
      return false;
    if (!(in >> s.seal))
      return false;
#if defined(CLONE_SCALING)
    if (!(in >> s.seen))
      return false;
#endif

    table_[index(s.hash)] = s;
  }

  seal_ = t_seal;
  probes_ = t_probes;
  hits_ = t_hits;

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if the object was saved correctly.
///
bool cache::save(std::ostream &out) const
{
  out << seal_ << ' ' << probes_ << ' ' << hits_ << '\n';

  std::size_t num(0);
  for (const auto &s : table_)
    if (!s.hash.empty())
      ++num;
  out << num << '\n';

  for (const auto &s : table_)
    if (!s.hash.empty())
    {
      s.hash.save(out);
      s.fitness.save(out);
      out << s.seal;
#if defined(CLONE_SCALING)
      out << ' ' << s.seen;
#endif
      out << '\n';
    }

  return out.good();
}

///
/// \return `true` if the object passes the internal consistency check.
///
bool cache::debug() const
{
  return probes() >= hits();
}

///
/// \param[out] o output stream.
/// \param[in] h hash signature to be printed.
///
/// Mainly useful for debugging / testing.
///
std::ostream &operator<<(std::ostream &o, hash_t h)
{
  return o << h.data[0] << h.data[1];
}
}  // namespace vita
