/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/team.h"

#include "test/fixture1.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("TEAM")
{

TEST_CASE_FIXTURE(fixture1, "Random creation")
{
  // Variable length random creation
  for (auto l(prob.sset.categories() + 2); l < 100; ++l)
  {
    prob.env.mep.code_length = l;
    vita::team<vita::i_mep> t(prob);

    CHECK(t.is_valid());
    CHECK(t.age() == 0);
  }
}

TEST_CASE_FIXTURE(fixture1, "Mutation")
{
  prob.env.mep.code_length = 100;

  vita::team<vita::i_mep> t(prob);
  const auto orig(t);

  CHECK(t.individuals() > 0);

  const unsigned n(4000);

  // Zero probability mutation
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation(0.0, prob);
    CHECK(t == orig);
  }

  // 50% probability mutation

  double diff(0.0), length(0.0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::team<vita::i_mep> t1{t};

    t.mutation(0.5, prob);
    diff += distance(t, t1);
    length += t1.active_symbols();
  }

  const double perc(100.0 * diff / length);
  CHECK(perc > 47.0);
  CHECK(perc < 52.0);
}

TEST_CASE_FIXTURE(fixture1, "Comparison")
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::team<vita::i_mep> a(prob);
    CHECK(a == a);
    CHECK(distance(a, a) == 0);

    vita::team<vita::i_mep> b(a);
    CHECK(a.signature() == b.signature());
    CHECK(a == b);
    CHECK(distance(a, b) == 0);

    vita::team<vita::i_mep> c(prob);
    if (a.signature() != c.signature())
    {
      CHECK(a != c);
      CHECK(distance(a, c) > 0);
    }
  }
}

TEST_CASE_FIXTURE(fixture1, "Iterators")
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::team<vita::i_mep> t(prob);

    unsigned i(0);
    for (const auto &ind : t)
    {
      CHECK(ind == t[i]);
      ++i;
    }
  }
}

TEST_CASE_FIXTURE(fixture1, "Crossover")
{
  using namespace vita;

  prob.env.mep.code_length = 100;

  team<vita::i_mep> t1(prob), t2(prob);

  const unsigned n(2000);
  for (unsigned j(0); j < n; ++j)
  {
    const auto tc(crossover(t1, t2));
    CHECK(tc.is_valid());

    for (unsigned x(0); x < tc.individuals(); ++x)
      for (index_t i(0); i != tc[x].size(); ++i)
        for (category_t c(0); c < tc[x].categories(); ++c)
        {
          const locus l{i, c};

          CHECK((tc[x][l] == t1[x][l] || tc[x][l] == t2[x][l]));
        }
  }
}

TEST_CASE_FIXTURE(fixture1, "Serialization")
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::team<vita::i_mep> t1(prob);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      t1.inc_age();

    CHECK(t1.save(ss));

    vita::team<vita::i_mep> t2(prob);
    CHECK(t2.load(ss, prob.sset));
    CHECK(t2.is_valid());

    CHECK(t1 == t2);
  }
}

}  // TEST_SUITE("TEAM")
