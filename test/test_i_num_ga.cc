/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/i_num_ga.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_i_num_ga
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr double epsilon(0.00001);

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_i_num_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Random creation");
  for (unsigned i(1); i < 1000; ++i)
  {
    vita::i_num_ga ind(env, sset);

    BOOST_REQUIRE(ind.debug());
    BOOST_REQUIRE_EQUAL(ind.size(), sset.categories());
    BOOST_REQUIRE_EQUAL(ind.age(), 0);
  }
}

BOOST_AUTO_TEST_CASE(Mutation)
{
  vita::i_num_ga t(env, sset);
  const vita::i_num_ga orig(t);

  const unsigned n(1000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation");
  env.p_mutation = 0.0;
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation();
    BOOST_REQUIRE_EQUAL(t, orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  env.p_mutation = 0.5;
  unsigned diff(0);

  for (unsigned i(0); i < n; ++i)
  {
    auto i1(orig);

    i1.mutation();
    diff += orig.distance(i1);
  }

  const double perc(100.0 * double(diff) / double(orig.size() * n));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_num_ga a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);
    BOOST_REQUIRE_EQUAL(a.distance(a), 0);

    vita::i_num_ga b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());
    BOOST_REQUIRE_EQUAL(a, b);
    BOOST_REQUIRE_EQUAL(a.distance(b), 0);

    vita::i_num_ga c(env, sset);
    if (a.signature() != c.signature())
    {
      BOOST_REQUIRE_NE(a, c);
      BOOST_REQUIRE_GT(a.distance(c), 0);
      BOOST_REQUIRE_EQUAL(a.distance(c), c.distance(a));
    }
  }
}

BOOST_AUTO_TEST_CASE(Crossover)
{
  vita::i_num_ga t1(env, sset), t2(env, sset);

  BOOST_TEST_CHECKPOINT("Standard crossover");
  double dist(0.0);
  const unsigned n(1000);
  for (unsigned j(0); j < n; ++j)
  {
    const auto tc(t1.crossover(t2));
    BOOST_CHECK(tc.debug(true));

    dist += t1.distance(tc);
  }

  // +1 since we have at least one gene involved in crossover.
  const double perc(100.0 * dist / ((sset.categories() + 1) * n));
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);

  BOOST_TEST_CHECKPOINT("DE crossover");
  for (unsigned j(0); j < n; ++j)
  {
    const vita::i_num_ga base(env, sset);

    t1 = t2;
    auto off(base.crossover(t1, t2));

    for (unsigned i(0); i < base.size(); ++i)
      BOOST_CHECK_CLOSE(off(i), base(i), epsilon);
  }
}

/*
BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::team<vita::i_mep> t1(env, sset);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      t1.inc_age();

    BOOST_REQUIRE(t1.save(ss));

    vita::team<vita::i_mep> t2(env, sset);
    BOOST_REQUIRE(t2.load(ss));
    BOOST_REQUIRE(t2.debug());

    BOOST_CHECK_EQUAL(t1, t2);
  }
}*/
BOOST_AUTO_TEST_SUITE_END()
