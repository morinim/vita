/**
 *
 *  \file test_team.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <sstream>

#include "kernel/team.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE team
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_FIXTURE_TEST_SUITE(team, F_FACTORY1)

BOOST_AUTO_TEST_CASE(Mutation)
{
  env.code_length = 100;

  vita::basic_team<vita::individual> t(env, sset);
  const vita::basic_team<vita::individual> orig(t);

  const unsigned n(4000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation.");
  env.p_mutation = 0.0;
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation();
    BOOST_REQUIRE_EQUAL(t, orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  env.p_mutation = 0.5;
  double diff(0.0), avg_length(0.0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::basic_team<vita::individual> t1(t);

    t.mutation();
    diff += t1.distance(t);
    avg_length += static_cast<double>(t1.eff_size()) / t1.individuals();
  }

  const double perc(100.0 * static_cast<double>(diff) / avg_length);
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation.");
  for (unsigned l(sset.categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::basic_team<vita::individual> i(env, sset);
    // std::cout << i << std::endl;

    BOOST_REQUIRE(i.debug());
    BOOST_REQUIRE_EQUAL(i.size(), l * i.individuals());
  }
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::basic_team<vita::individual> a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);

    vita::basic_team<vita::individual> b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());

    vita::basic_team<vita::individual> c(env, sset);
    if (a.signature() != c.signature())
      BOOST_REQUIRE_NE(a, c);
  }
}

BOOST_AUTO_TEST_CASE(Crossover)
{
  env.code_length = 100;

  vita::basic_team<vita::individual> i1(env, sset), i2(env, sset);

  const unsigned n(2000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
    dist += i1.distance(i1.crossover(i2));

  const double perc(100.0 * dist / (env.code_length * sset.categories() * n));
  BOOST_CHECK_GT(perc, 45.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::basic_team<vita::individual> t1(env, sset);

    const auto sup(vita::random::between(0u, 100u));
    for (unsigned j(0); j < sup; ++j)
      t1.inc_age();

    BOOST_REQUIRE(t1.save(ss));

    vita::basic_team<vita::individual> t2(env, sset);
    BOOST_REQUIRE(t2.load(ss));
    BOOST_REQUIRE(t2.debug());

    BOOST_CHECK_EQUAL(t1, t2);
  }
}

BOOST_AUTO_TEST_SUITE_END()
