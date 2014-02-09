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

#include <cstdlib>
#include <sstream>

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"
#include "kernel/src/primitive/factory.h"
#include "kernel/ttable_hash.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE TranspositionTable
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture2.h"
#endif


BOOST_AUTO_TEST_SUITE(hash)

// This should hopefully be a thorough and uambiguous test of whether the hash
// is correctly implemented.
BOOST_AUTO_TEST_CASE(MurmurHash)
{
  const unsigned hashbytes(128 / 8);

  std::uint8_t *const key(new std::uint8_t[256]);
  std::uint8_t *const hashes(new std::uint8_t[hashbytes * 256]);
  std::uint8_t *const final(new std::uint8_t[hashbytes]);

  std::memset(key, 0, 256);
  std::memset(hashes, 0, hashbytes * 256);
  std::memset(final, 0, hashbytes);

  // Hash keys of the form {0}, {0,1}, {0,1,2}... up to N=255, using 256-N as
  // the seed.
  for (unsigned i(0); i < 256; ++i)
  {
    key[i] = static_cast<std::uint8_t>(i);

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
  const std::uint32_t verification((final[0] <<  0) | (final[1] <<  8) |
                                   (final[2] << 16) | (final[3] << 24));

  delete [] key;
  delete [] hashes;
  delete [] final;

  //----------

  BOOST_CHECK_EQUAL(0x6384BA69, verification);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE(ttable, F_FACTORY2)

BOOST_AUTO_TEST_CASE(InsertFindCicle)
{
  vita::ttable cache(16);
  env.code_length = 64;

  const unsigned n(6000);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::individual i1(env, sset);
    const auto base_f(static_cast<vita::fitness_t::base_t>(i));
    vita::fitness_t f({base_f});

    cache.insert(i1.signature(), f);

    BOOST_REQUIRE(cache.find(i1.signature(), &f));
    BOOST_REQUIRE_EQUAL(f, vita::fitness_t{base_f});
  }
}

BOOST_AUTO_TEST_CASE(CollisionDetection)
{
  using i_interp = vita::interpreter<vita::individual>;
  vita::ttable cache(14);
  env.code_length = 64;

  const unsigned n(1000);

  std::vector<vita::individual> vi;
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, sset);
    const vita::any val(i_interp(i1).run());
    vita::fitness_t f(
      {val.empty() ? 0.0 : vita::any_cast<vita::fitness_t::base_t>(val)});

    cache.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::fitness_t f;
    if (cache.find(vi[i].signature(), &f))
    {
      const vita::any val(i_interp(vi[i]).run());
      vita::fitness_t f1(
        {val.empty() ? 0.0 : vita::any_cast<vita::fitness_t::base_t>(val)});

      BOOST_CHECK_EQUAL(f, f1);
    }
  }
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  using i_interp = vita::interpreter<vita::individual>;
  vita::ttable cache(14), cache2(14);
  env.code_length = 64;

  const unsigned n(1000);
  std::vector<vita::individual> vi;
  std::vector<bool> present(n);

  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, sset);
    const vita::any val(i_interp(i1).run());
    vita::fitness_t f(
      {val.empty() ? 0.0 : vita::any_cast<vita::fitness_t::base_t>(val)});

    cache.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::fitness_t f;
    present[i] = cache.find(vi[i].signature(), &f);
  }

  std::stringstream ss;
  BOOST_REQUIRE(cache.save(ss));

  BOOST_REQUIRE(cache2.load(ss));

  for (unsigned i(0); i < n; ++i)
    if (present[i])
    {
      const vita::any val(i_interp(vi[i]).run());
      vita::fitness_t f(
        {val.empty() ? 0.0 : vita::any_cast<vita::fitness_t::base_t>(val)});

      vita::fitness_t f1;
      BOOST_CHECK(cache2.find(vi[i].signature(), &f1));

      BOOST_CHECK_EQUAL(f, f1);
    }
}

BOOST_AUTO_TEST_SUITE_END()
