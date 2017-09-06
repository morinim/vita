/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/cache.h"
#include "kernel/environment.h"
#include "kernel/i_mep.h"
#include "kernel/interpreter.h"
#include "kernel/src/primitive/factory.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE TranspositionTable
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture2.h"
#endif


BOOST_AUTO_TEST_SUITE(test_hash)

// This should hopefully be a thorough and unambiguous test of whether the hash
// is correctly implemented.
BOOST_AUTO_TEST_CASE(MurmurHash)
{
  using vita::hash_t;

  const unsigned hashbytes(128 / 8);

  using byte = unsigned char;

  byte *const key(new byte[256]);
  byte *const hashes(new byte[hashbytes * 256]);
  byte *const final(new byte[hashbytes]);

  std::memset(key, 0, 256);
  std::memset(hashes, 0, hashbytes * 256);
  std::memset(final, 0, hashbytes);

  // Hash keys of the form {0}, {0,1}, {0,1,2}... up to N=255, using 256-N as
  // the seed.
  for (unsigned i(0); i < 256; ++i)
  {
    key[i] = static_cast<byte>(i);

    auto h(vita::hash::hash128(key, i, 256 - i));
    reinterpret_cast<std::uint64_t *>(&hashes[i * hashbytes])[0] = h.data[0];
    reinterpret_cast<std::uint64_t *>(&hashes[i * hashbytes])[1] = h.data[1];
  }

  // Then hash the result array.
  auto h(vita::hash::hash128(hashes, hashbytes * 256, 0));
  reinterpret_cast<std::uint64_t *>(final)[0] = h.data[0];
  reinterpret_cast<std::uint64_t *>(final)[1] = h.data[1];

  // The first four bytes of that hash, interpreted as a little-endian integer,
  // is our verification value.
  const auto verification((static_cast<std::uint32_t>(final[0]) <<  0u) |
                          (static_cast<std::uint32_t>(final[1]) <<  8u) |
                          (static_cast<std::uint32_t>(final[2]) << 16u) |
                          (static_cast<std::uint32_t>(final[3]) << 24u));

  delete [] key;
  delete [] hashes;
  delete [] final;

  //----------

  BOOST_CHECK(0x6384BA69 == verification);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE(test_cache, F_FACTORY2)

BOOST_AUTO_TEST_CASE(InsertFindCicle)
{
  vita::cache cache(16);
  env.code_length = 64;

  const unsigned n(6000);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::i_mep i1(env);
    const auto base_f(static_cast<vita::fitness_t::value_type>(i));
    vita::fitness_t f{base_f};

    cache.insert(i1.signature(), f);

    BOOST_CHECK(cache.find(i1.signature()) == f);
  }
}

BOOST_AUTO_TEST_CASE(CollisionDetection)
{
  using i_interp = vita::interpreter<vita::i_mep>;
  vita::cache cache(14);
  env.code_length = 64;

  const unsigned n(1000);

  std::vector<vita::i_mep> vi;
  for (unsigned i(0); i < n; ++i)
  {
    vita::i_mep i1(env);
    const vita::any val(i_interp(&i1).run());
    vita::fitness_t f{val.has_value()
                      ? vita::any_cast<vita::fitness_t::value_type>(val) : 0.0};

    cache.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    const vita::fitness_t f(cache.find(vi[i].signature()));
    if (f.size())
    {
      const vita::any val(i_interp(&vi[i]).run());
      vita::fitness_t f1{val.has_value()
                         ? vita::any_cast<vita::fitness_t::value_type>(val)
                         : 0.0};

      BOOST_CHECK(f == f1);
    }
  }
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  using namespace vita;

  using i_interp = interpreter<i_mep>;
  vita::cache cache1(14), cache2(14);
  env.code_length = 64;

  const unsigned n(1000);
  std::vector<i_mep> vi;
  std::vector<bool> present(n);

  for (unsigned i(0); i < n; ++i)
  {
    i_mep i1(env);
    const vita::any val(i_interp(&i1).run());
    fitness_t f{val.has_value() ? any_cast<fitness_t::value_type>(val) : 0.0};

    cache1.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  std::transform(vi.begin(), vi.end(), present.begin(),
                 [&cache1](const i_mep &i)
                 {
                   return cache1.find(i.signature()).size();
                 });

  std::stringstream ss;
  BOOST_CHECK(cache1.save(ss));

  BOOST_CHECK(cache2.load(ss));

  for (unsigned i(0); i < n; ++i)
    if (present[i])
    {
      const vita::any val(i_interp(&vi[i]).run());
      fitness_t f{val.has_value()
                  ? any_cast<fitness_t::value_type>(val) : 0.0};

      fitness_t f1(cache2.find(vi[i].signature()));
      BOOST_CHECK(f1.size());
      BOOST_CHECK(f == f1);
    }
}

BOOST_AUTO_TEST_CASE(HashT)
{
  const vita::hash_t empty;
  BOOST_CHECK(empty.empty());

  vita::hash_t h(123, 345);
  BOOST_CHECK(!h.empty());

  BOOST_CHECK(h != empty);

  h.clear();
  BOOST_CHECK(h.empty());

  BOOST_CHECK(h == empty);
}

BOOST_AUTO_TEST_SUITE_END()
