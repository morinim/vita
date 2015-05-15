/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/environment.h"
#include "kernel/i_mep.h"
#include "kernel/interpreter.h"
#include "kernel/src/primitive/factory.h"
#include "kernel/ttable_hash.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE TranspositionTable
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture2.h"
#endif


BOOST_AUTO_TEST_SUITE(test_hash)

// This should hopefully be a thorough and uambiguous test of whether the hash
// is correctly implemented.
BOOST_AUTO_TEST_CASE(MurmurHash)
{
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

    vita::hash_t h(vita::hash(key, i, 256 - i));
    reinterpret_cast<std::uint64_t *>(&hashes[i * hashbytes])[0] = h.data[0];
    reinterpret_cast<std::uint64_t *>(&hashes[i * hashbytes])[1] = h.data[1];
  }

  // Then hash the result array.
  vita::hash_t h(vita::hash(hashes, hashbytes * 256, 0));
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

  BOOST_CHECK_EQUAL(0x6384BA69, verification);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE(test_ttable, F_FACTORY2)

BOOST_AUTO_TEST_CASE(InsertFindCicle)
{
  vita::ttable cache(16);
  env.code_length = 64;

  const unsigned n(6000);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::i_mep i1(env);
    const auto base_f(static_cast<vita::fitness_t::value_type>(i));
    vita::fitness_t f{base_f};

    cache.insert(i1.signature(), f);

    BOOST_REQUIRE(cache.find(i1.signature(), &f));
    BOOST_REQUIRE_EQUAL(f, vita::fitness_t{base_f});
  }
}

BOOST_AUTO_TEST_CASE(CollisionDetection)
{
  using i_interp = vita::interpreter<vita::i_mep>;
  vita::ttable cache(14);
  env.code_length = 64;

  const unsigned n(1000);

  std::vector<vita::i_mep> vi;
  for (unsigned i(0); i < n; ++i)
  {
    vita::i_mep i1(env);
    const vita::any val(i_interp(&i1).run());
    vita::fitness_t f{val.empty() ?
        0.0 : vita::any_cast<vita::fitness_t::value_type>(val)};

    cache.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::fitness_t f;
    if (cache.find(vi[i].signature(), &f))
    {
      const vita::any val(i_interp(&vi[i]).run());
      vita::fitness_t f1{
        val.empty() ? 0.0 : vita::any_cast<vita::fitness_t::value_type>(val)};

      BOOST_CHECK_EQUAL(f, f1);
    }
  }
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  using namespace vita;

  using i_interp = interpreter<i_mep>;
  ttable cache(14), cache2(14);
  env.code_length = 64;

  const unsigned n(1000);
  std::vector<i_mep> vi;
  std::vector<bool> present(n);

  for (unsigned i(0); i < n; ++i)
  {
    i_mep i1(env);
    const vita::any val(i_interp(&i1).run());
    fitness_t f{val.empty() ? 0.0 : any_cast<fitness_t::value_type>(val)};

    cache.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    fitness_t f;
    present[i] = cache.find(vi[i].signature(), &f);
  }

  std::stringstream ss;
  BOOST_REQUIRE(cache.save(ss));

  BOOST_REQUIRE(cache2.load(ss));

  for (unsigned i(0); i < n; ++i)
    if (present[i])
    {
      const vita::any val(i_interp(&vi[i]).run());
      fitness_t f{val.empty() ? 0.0 : any_cast<fitness_t::value_type>(val)};

      fitness_t f1(f.size(), fit_tag::components);
      BOOST_CHECK(cache2.find(vi[i].signature(), &f1));

      BOOST_CHECK_EQUAL(f, f1);
    }
}

BOOST_AUTO_TEST_CASE(HashT)
{
  vita::hash_t empty;
  BOOST_REQUIRE(empty.empty());

  vita::hash_t h(123, 345);
  BOOST_REQUIRE(!h.empty());

  BOOST_REQUIRE_NE(empty, h);

  h.clear();
  BOOST_REQUIRE(h.empty());

  BOOST_REQUIRE_EQUAL(empty, h);
}

BOOST_AUTO_TEST_SUITE_END()
