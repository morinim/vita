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
#define BOOST_TEST_MODULE num_ga
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(num_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Random creation");
  for (unsigned i(1); i < 1000; ++i)
  {
    vita::i_num_ga ind(env, sset);

    BOOST_REQUIRE(ind.debug());
    BOOST_REQUIRE_EQUAL(ind.age(), 0);
  }
}
/*
BOOST_AUTO_TEST_CASE(Mutation)
{
  env.code_length = 100;

  vita::team<vita::i_mep> t(env, sset);
  const vita::team<vita::i_mep> orig(t);

  BOOST_REQUIRE_GT(t.individuals(), 0);

  const unsigned n(4000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation");
  env.p_mutation = 0.0;
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation();
    BOOST_REQUIRE_EQUAL(t, orig);
  }
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::team<vita::i_mep> a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);

    vita::team<vita::i_mep> b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());

    vita::team<vita::i_mep> c(env, sset);
    if (a.signature() != c.signature())
      BOOST_REQUIRE_NE(a, c);
  }
}

BOOST_AUTO_TEST_CASE(Crossover)
{
  env.code_length = 100;

  vita::team<vita::i_mep> t1(env, sset), t2(env, sset);

  const unsigned n(2000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
  {
    const auto tc(t1.crossover(t2));
    BOOST_CHECK(tc.debug(true));

    dist += t1.distance(tc);
  }

  const double perc(100.0 * dist /
                    (env.code_length * sset.categories() * n *
                     t1.individuals()));
  BOOST_CHECK_GT(perc, 45.0);
  BOOST_CHECK_LT(perc, 52.0);
}

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
