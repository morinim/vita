/**
 *
 *  \file test_individual.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <sstream>

#include "kernel/individual.h"
#include "kernel/interpreter.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE individual
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_FIXTURE_TEST_SUITE(individual, F_FACTORY1)
/*
BOOST_AUTO_TEST_CASE(Compact)
{
  env.code_length = 100;

  std::cout << env.sset << std::endl;

  BOOST_TEST_CHECKPOINT("Functional equivalence.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::individual i1(env, true);
    const vita::individual i2(i1.compact());

    std::cout << i1 << std::endl << i2 << std::endl;
    BOOST_REQUIRE(i1.debug(true));

    const boost::any v1( (vita::interpreter(i1))() );
    const boost::any v2( (vita::interpreter(i2))() );

    BOOST_REQUIRE_EQUAL(v1.empty(), v2.empty());
    if (!v1.empty() && !v2.empty())
      BOOST_REQUIRE_EQUAL(vita::interpreter::to_string(v1),
                          vita::interpreter::to_string(v2));
  }

  BOOST_TEST_CHECKPOINT("Not interleaved active symbols.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::individual ind(vita::individual(env, true).compact());

    unsigned line(0), old_line(0);
    for (vita::individual::const_iterator it(ind); it(); line = ++it)
      if (line)
      {
        BOOST_REQUIRE_EQUAL(old_line, line-1);
        ++old_line;
      }
  }

  BOOST_TEST_CHECKPOINT("Same signature.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::individual i1(env, true);
    const vita::individual i2(i1.compact());

    BOOST_REQUIRE_EQUAL(i1.signature(), i2.signature());
  }
}
*/

BOOST_AUTO_TEST_CASE(Mutation)
{
  env.code_length = 100;

  vita::individual ind(env, sset);
  const vita::individual orig(ind);

  const unsigned n(4000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation.");
  env.p_mutation = 0.0;
  for (unsigned i(0); i < n; ++i)
  {
    ind.mutation();
    BOOST_REQUIRE_EQUAL(ind, orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  env.p_mutation = 0.5;
  unsigned diff(0), length(0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::individual i1(ind);

    ind.mutation();
    diff += i1.distance(ind);
    length += i1.eff_size();
  }

  const double perc(100.0 * double(diff) / double(length));
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation.");
  for (unsigned l(sset.categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::individual i(env, sset);
    // std::cout << i << std::endl;

    BOOST_REQUIRE(i.debug());
    BOOST_REQUIRE_EQUAL(i.size(), l);
  }
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::individual a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);

    vita::individual b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());

    vita::individual c(env, sset);
    if (a.signature() != c.signature())
      BOOST_REQUIRE_NE(a, c);
  }
}

BOOST_AUTO_TEST_CASE(Crossover)
{
  env.code_length = 100;

  vita::individual i1(env, sset), i2(env, sset);

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
    vita::individual i1(env, sset);

    const auto sup(vita::random::between(0u, 100u));
    for (unsigned j(0); j < sup; ++j)
      i1.inc_age();

    BOOST_REQUIRE(i1.save(ss));

    vita::individual i2(env, sset);
    BOOST_REQUIRE(i2.load(ss));
    BOOST_REQUIRE(i2.debug());

    BOOST_CHECK_EQUAL(i1, i2);
  }
}

BOOST_AUTO_TEST_SUITE_END()
