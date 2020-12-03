/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2018-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

#include "kernel/cache.h"
#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/mep/interpreter.h"
#include "kernel/gp/src/primitive/factory.h"
#include "kernel/problem.h"

#include "test/fixture2.h"


TEST_SUITE("CACHE")
{

// This should hopefully be a thorough and unambiguous test of whether the hash
// is correctly implemented.
TEST_CASE("Murmur Hash")
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

  CHECK(verification == 0x6384BA69);
}  // TEST_CASE("Murmur Hash")


TEST_CASE_FIXTURE(fixture2, "Insert/Find cycle")
{
  vita::cache cache(16);
  prob.env.mep.code_length = 64;

  const unsigned n(6000);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::i_mep i1(prob);
    const auto base_f(static_cast<vita::fitness_t::value_type>(i));
    vita::fitness_t f{base_f};

    cache.insert(i1.signature(), f);

    CHECK(cache.find(i1.signature()) == f);
  }
}

TEST_CASE_FIXTURE(fixture2, "Collision detection")
{
  using namespace vita;
  cache cache(14);
  prob.env.mep.code_length = 64;

  const unsigned n(1000);

  std::vector<i_mep> vi;
  for (unsigned i(0); i < n; ++i)
  {
    i_mep i1(prob);
    const auto val(run(i1));
    fitness_t f{vita::has_value(val) ? std::get<D_DOUBLE>(val) : 0.0};

    cache.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    const fitness_t f(cache.find(vi[i].signature()));
    if (f.size())
    {
      const auto val(run(vi[i]));
      fitness_t f1{has_value(val) ? std::get<D_DOUBLE>(val) : 0.0};

      CHECK(f == f1);
    }
  }
}

TEST_CASE_FIXTURE(fixture2, "Serialization")
{
  using namespace vita;

  vita::cache cache1(14), cache2(14);
  prob.env.mep.code_length = 64;

  const unsigned n(1000);
  std::vector<i_mep> vi;
  std::vector<bool> present(n);

  for (unsigned i(0); i < n; ++i)
  {
    i_mep i1(prob);
    const auto val(run(i1));
    fitness_t f{vita::has_value(val) ? std::get<D_DOUBLE>(val) : 0.0};

    cache1.insert(i1.signature(), f);
    vi.push_back(i1);
  }

  std::transform(vi.begin(), vi.end(), present.begin(),
                 [&cache1](const i_mep &i)
                 {
                   return cache1.find(i.signature()).size();
                 });

  std::stringstream ss;
  CHECK(cache1.save(ss));

  CHECK(cache2.load(ss));

  for (unsigned i(0); i < n; ++i)
    if (present[i])
    {
      const auto val(run(vi[i]));
      fitness_t f{vita::has_value(val) ? std::get<D_DOUBLE>(val) : 0.0};

      fitness_t f1(cache2.find(vi[i].signature()));
      CHECK(f1.size());
      CHECK(f == f1);
    }
}

TEST_CASE("Type hash_t")
{
  const vita::hash_t empty;
  CHECK(empty.empty());

  vita::hash_t h(123, 345);
  CHECK(!h.empty());

  CHECK(h != empty);

  h.clear();
  CHECK(h.empty());

  CHECK(h == empty);
}

}  // TEST_SUITE("CACHE")
