/**
 *
 *  \file ttable.cc
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

#include "kernel/ttable.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \param[in] bits 2^\a bits is the number of elements of the table.
  ///
  /// Creates a new transposition (hash) table.
  ///
  ttable::ttable(unsigned bits)
    : k_mask((1 << bits)-1), table_(new slot[1 << bits]), probes_(0), hits_(0)
  {
    assert(check());
  }

  ///
  ttable::~ttable()
  {
    delete [] table_;
  }

  ///
  /// Clears the content and the statistical informations of the table
  /// (allocated size isn't changed).
  ///
  void ttable::clear()
  {
    probes_ = 0;
    hits_   = 0;

    for (unsigned i(0); i <= k_mask; ++i)
    {
      table_[i].hash = hash_t();
      table_[i].fit  = 0;
    }
  }

  ///
  /// \param[in] ind the individual to look for.
  /// \param[out] fit the fitness of the individual (if found).
  /// \return \c true if \a ind is found in the transposition table, \c false
  ///         otherwise.
  ///
  /// Looks for the fitness of an individual in the transposition table.
  ///
  bool ttable::find(const individual &ind, fitness_t *const fit) const
  {
    ++probes_;

    std::vector<boost::uint8_t> packed;
    ind.pack(&packed);

    const hash_t h(hash(packed));

    const slot &s(table_[h.p1 & k_mask]);

    const bool ret(h == s.hash);

    if (ret)
    {
      ++hits_;
      *fit = s.fit;
    }

    return ret;
  }

  ///
  /// \param[in] ind a new individual to be stored in the table.
  /// \param[out] fit the fitness of the individual.
  ///
  /// Stores fitness information in the transposition table.
  ///
  void ttable::insert(const individual &ind, fitness_t fit)
  {
    std::vector<boost::uint8_t> packed;
    ind.pack(&packed);

    slot s;
    s.hash = hash(packed);
    s.fit  =          fit;

    table_[s.hash.p1 & k_mask] = s;
  }

  ///
  /// \param[in] packed a byte level representation of an \a individual.
  ///
  /// MurmurHash3 (<http://code.google.com/p/smhasher/>), by Austin Appleby.
  /// This is a relatively simple non-cryptographic hash algorithm. It is
  /// noted for being fast, with excellent distribution, avalanche behavior and
  ///  overall collision resistance.
  ///
  ttable::hash_t ttable::hash(const std::vector<boost::uint8_t> &packed)
  {
    const unsigned len(packed.size());
    const unsigned n_blocks(len / 16);

    const boost::uint64_t seed(1973);
    boost::uint64_t h1(seed);
    boost::uint64_t h2(seed);

    boost::uint64_t c1(0x87c37b91114253d5);
    boost::uint64_t c2(0x4cf5ad432745937f);

    const boost::uint64_t *const blocks(
      reinterpret_cast<const boost::uint64_t *>(&packed[0]));

    for (unsigned i(0); i < n_blocks; ++i)
    {
      boost::uint64_t k1(blocks[i*2+0]);
      boost::uint64_t k2(blocks[i*2+1]);

      k1 *= c1;
      k1  = ROTL64(k1, 31);
      k1 *= c2;
      h1 ^= k1;

      h1 = ROTL64(h1, 27);
      h1 += h2;
      h1 = h1*5 + 0x52dce729;

      k2 *= c2;
      k2  = ROTL64(k2, 33);
      k2 *= c1;
      h2 ^= k2;

      h2 = ROTL64(h2, 31);
      h2 += h1;
      h2 = h2*5 + 0x38495ab5;
    }

    const boost::uint8_t *tail(
      static_cast<const boost::uint8_t *>(&packed[0] + n_blocks*16));

    boost::uint64_t k1(0);
    boost::uint64_t k2(0);

    switch (len & 15)
    {
    case 15: k2 ^= boost::uint64_t(tail[14]) << 48;
    case 14: k2 ^= boost::uint64_t(tail[13]) << 40;
    case 13: k2 ^= boost::uint64_t(tail[12]) << 32;

    case 12: k2 ^= boost::uint64_t(tail[11]) << 24;
    case 11: k2 ^= boost::uint64_t(tail[10]) << 16;
    case 10: k2 ^= boost::uint64_t(tail[ 9]) << 8;
    case  9:
      k2 ^= boost::uint64_t(tail[ 8]) << 0;
      k2 *= c2;
      k2  = ROTL64(k2, 33);
      k2 *= c1;
      h2 ^= k2;

    case  8: k1 ^= boost::uint64_t(tail[ 7]) << 56;
    case  7: k1 ^= boost::uint64_t(tail[ 6]) << 48;
    case  6: k1 ^= boost::uint64_t(tail[ 5]) << 40;
    case  5: k1 ^= boost::uint64_t(tail[ 4]) << 32;
    case  4: k1 ^= boost::uint64_t(tail[ 3]) << 24;
    case  3: k1 ^= boost::uint64_t(tail[ 2]) << 16;
    case  2: k1 ^= boost::uint64_t(tail[ 1]) << 8;
    case  1:
      k1 ^= boost::uint64_t(tail[ 0]) << 0;
      k1 *= c1;
      k1  = ROTL64(k1, 31);
      k1 *= c2;
      h1 ^= k1;
    }

    h1 ^= len;
    h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    h2 ^= h2 >> 16;
    h2 *= 0x85ebca6b;
    h2 ^= h2 >> 13;
    h2 *= 0xc2b2ae35;
    h2 ^= h2 >> 16;

    h1 += h2;
    h2 += h1;

    return hash_t(h1, h2);
  }

  ///
  /// \return \c true if the table passes the internal consistency check.
  ///
  bool ttable::check() const
  {
    return probes() >= hits();
  }
}  // namespace vita
