/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/i_mep.h"
#include "kernel/team.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE team
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_FIXTURE_TEST_SUITE(team, F_FACTORY1)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation");
  for (unsigned l(sset.categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::team<vita::i_mep> t(env, sset);
    // std::cout << t << std::endl;

    BOOST_REQUIRE(t.debug());
    BOOST_REQUIRE_EQUAL(t.age(), 0);
  }
}

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
/*
  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  env.p_mutation = 0.5;

  double diff(0.0), avg_length(0.0);

  for (unsigned i(0); i < n; ++i)
  {
    const auto t1{t};

    t.mutation();
    diff += t.distance(t1);
    length += t1.eff_size();
  }

  const double perc(100.0 * diff / length);
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 52.0);*/
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::team<vita::i_mep> a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);
    BOOST_REQUIRE_EQUAL(a.distance(a), 0);

    vita::team<vita::i_mep> b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());
    BOOST_REQUIRE_EQUAL(a, b);
    BOOST_REQUIRE_EQUAL(a.distance(b), 0);

    vita::team<vita::i_mep> c(env, sset);
    if (a.signature() != c.signature())
    {
      BOOST_REQUIRE_NE(a, c);
      BOOST_REQUIRE_GT(a.distance(c), 0);
    }
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
}
BOOST_AUTO_TEST_SUITE_END()
