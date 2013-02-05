/**
 *
 *  \file test_ttable.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <sstream>

#include "environment.h"
#include "individual.h"
#include "interpreter.h"
#include "primitive/factory.h"
#include "ttable_hash.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE TranspositionTable
#include "boost/test/unit_test.hpp"

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

    vita::hash(key, i, 256 - i, &hashes[i * hashbytes]);
  }

  // Then hash the result array.
  vita::hash(hashes, hashbytes * 256, 0, final);

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
    vita::individual i1(env, true);
    vita::score_t s(i, 0.0);

    cache.insert(i1, s);

    BOOST_REQUIRE(cache.find(i1, &s));
    BOOST_REQUIRE_EQUAL(s.fitness, i);
  }
}

BOOST_AUTO_TEST_CASE(CollisionDetection)
{
  vita::ttable cache(14);
  env.code_length = 64;

  const unsigned n(1000);

  std::vector<vita::individual> vi;
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, true);
    const vita::any val(vita::interpreter(i1).run());
    vita::fitness_t f(val.empty() ? 0 : vita::any_cast<vita::fitness_t>(val));
    vita::score_t s(f, 0.0);

    cache.insert(i1, s);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::score_t s;
    if (cache.find(vi[i], &s))
    {
      const vita::any val(vita::interpreter(vi[i]).run());
      vita::fitness_t f(val.empty() ? 0 : vita::any_cast<vita::fitness_t>(val));

      BOOST_CHECK_EQUAL(s.fitness, f);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
