/**
 *
 *  \file ttable.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/09
 *
 *  This file is part of VITA
 *
 */

#include "ttable.h"

#include "individual.h"

namespace vita
{

  ///
  /// \param[in] bits 2^\a bits is the number of elements of the table.
  ///
  /// Creates a new transposition (hash) table.
  ///
  ttable::ttable(unsigned bits)
    : mask((1 << bits)-1), table(new slot[1 << bits]), _probes(0), _hits(0)
  {
    assert(check());
  }

  ///
  ttable::~ttable()
  {
    delete table;
  }

  ///
  /// Clears the content and the statistical informations of the table
  /// (allocated size isn't changed). 
  ///
  void
  ttable::clear()
  {
    _probes = 0;
    _hits   = 0;

    for (unsigned i(0); i <= mask; ++i)
    {
      table[i].hash = 0;
      table[i].fit  = 0;
    }
  }

  ///
  /// \param[in] ind the individual to look for.
  /// \param[out] fit the fitness of the individual (if found).
  /// \return true if \a ind is found in the transposition table, false 
  ///         otherwise.
  ///
  /// Looks for the fitness of an individual in the transposition table.
  ///
  bool
  ttable::find(const individual &ind, fitness_t *const fit) const
  {
    ++_probes;

    std::vector<boost::uint8_t> packed;
    ind.pack(packed);

    const hash_t h(hash(packed));

    const slot &s(table[h & mask]);

    const bool ret(h == s.hash);

    if (ret)
    {
      ++_hits;
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
  void
  ttable::insert(const individual &ind, fitness_t fit)
  {
    std::vector<boost::uint8_t> packed;

    ind.pack(packed);

    slot s;
    s.hash = hash(packed);
    s.fit  =          fit;
    
    table[s.hash & mask] = s; 
  }

  ///
  /// \param[in] packed a byte level representation of an individual.
  ///
  /// MurmurHash2, by Austin Appleby.
  /// This is a relatively simple hash algorithm. It is noted for being fast,
  /// with excellent distribution, avalanche behavior and overall collision
  /// resistance.
  ///
  ttable::hash_t
  ttable::hash(const std::vector<boost::uint8_t> &packed)
  {
    // m and r are mixing constants generated offline. They are not really
    // 'magic', they just happen to work well.
    const hash_t m(0xc6a4a7935bd1e995ull);
    const unsigned r(47);
    const unsigned seed(1973);
    
    const unsigned len(packed.size());

    // Initialize the hash to a 'random' value. 
    hash_t h(seed ^ len);

    // Mix 8 bytes at a time into the hash.
    const hash_t *data = (const hash_t *)&packed[0];
    const hash_t *const end = data + (len/8);

    while (data != end)
    {
      hash_t k(*data++);

      k *= m;
      k ^= k >> r;
      k *= m;

      h ^= k;
      h *= m;
    }

    // Handle the last few bytes of the input array.
    const boost::uint8_t *const data2 = (const boost::uint8_t *)data;

    switch (len&7)
    {
    case 7: h ^= hash_t(data2[6]) << 48;
    case 6: h ^= hash_t(data2[5]) << 40;
    case 5: h ^= hash_t(data2[4]) << 32;
    case 4: h ^= hash_t(data2[3]) << 24;
    case 3: h ^= hash_t(data2[2]) << 16;
    case 2: h ^= hash_t(data2[1]) <<  8;
    case 1: h ^= hash_t(data2[0]);
            h *= m;
    }

    // Do a few final mixes of the hash to ensure the last few bytes are
    // well-incorporated.
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    
    return h;
  } 

  ///
  /// \return true if the table passes the internal consistency check.
  ///
  bool
  ttable::check() const
  {
    return probes() >= hits();
  }

}  // namespace vita
