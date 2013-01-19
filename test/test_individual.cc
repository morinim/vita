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
#include <iostream>

#include "environment.h"
#include "individual.h"
#include "interpreter.h"
#include "primitive/factory.h"

#define BOOST_TEST_MODULE Individual
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F() : env(true)
  {
    BOOST_TEST_MESSAGE("Setup fixture");

    vita::symbol_factory &factory(vita::symbol_factory::instance());

    env.insert(factory.make("REAL", {0}));
    env.insert(factory.make("FADD", {0}));
    env.insert(factory.make("FSUB", {0}));
    env.insert(factory.make("FMUL", {0}));
    env.insert(factory.make("FDIV", {0}));
    env.insert(factory.make("FLN", {0}));
    env.insert(factory.make("FIFL", {0}));
    env.insert(factory.make("FIFZ", {0}));
    env.insert(factory.make("FIFE", {0, 0}));
    env.insert(factory.make("FSIN", {0}));
    env.insert(factory.make("FABS", {0}));
    env.insert(factory.make("FLENGTH", {1, 0}));

    env.insert(factory.make("apple", {1}));
    env.insert(factory.make("pear", {1}));
    env.insert(factory.make("grapefruit", {1}));
    env.insert(factory.make("orange", {1}));
    env.insert(factory.make("blueberry", {1}));
    env.insert(factory.make("blackberry", {1}));
    env.insert(factory.make("passion fruit", {1}));
    env.insert(factory.make("plum", {1}));
    env.insert(factory.make("date", {1}));
    env.insert(factory.make("peach", {1}));
    env.insert(factory.make("IFE", {1, 0}));
  }

  ~F()
  {
    BOOST_TEST_MESSAGE("Teardown fixture");
  }

  vita::environment env;
};

BOOST_FIXTURE_TEST_SUITE(Individual, F)
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
    BOOST_REQUIRE(i1.check(true));

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

  vita::individual ind(env, true);
  const vita::individual orig(ind);

  BOOST_TEST_CHECKPOINT("Zero probability mutation.");
  env.p_mutation = 0.0;

  for (unsigned i(0); i < 1000; ++i)
  {
    ind.mutation();
    BOOST_REQUIRE_EQUAL(ind, orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  env.p_mutation = 0.5;
  std::uint64_t diff(0), length(0);

  const unsigned n(2000);
  for (unsigned i(0); i < n; ++i)
  {
    const vita::individual i1(ind);

    ind.mutation();
    diff += i1.distance(ind);
    length += i1.eff_size();
  }

  const double perc(100.0 * double(diff) / double(length));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation.");
  for (unsigned l(env.sset.categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::individual i(env, true);
    // std::cout << i << std::endl;

    BOOST_REQUIRE(i.check());
    BOOST_REQUIRE_EQUAL(i.size(), l);
  }
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 1000; ++i)
  {
    vita::individual a(env, true);
    BOOST_REQUIRE_EQUAL(a, a);

    vita::individual b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());

    vita::individual c(env, true);
    if (!(a.signature() == c.signature()))
      BOOST_REQUIRE_NE(a, c);
  }
}

BOOST_AUTO_TEST_CASE(Cross0)
{
  env.code_length = 100;

  std::uint64_t diff(0), length(0);

  const unsigned n(1000);
  for (unsigned j(0); j < n; ++j)
  {
    const vita::individual i1(env, true), i2(env, true);
    const vita::individual off(uniform_crossover(i1, i2));

    diff += off.distance(i1);
    length += i1.eff_size();
  }

  const double perc(100.0 * double(diff) / double(length));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}

BOOST_AUTO_TEST_CASE(Cross1)
{
  env.code_length = 100;

  vita::individual i1(env, true), i2(env, true);

  const unsigned n(1000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
    dist += i1.distance(one_point_crossover(i1, i2));

  const double perc(100.0 * dist /
                    (*env.code_length * env.sset.categories() * n));
  BOOST_CHECK_GT(perc, 45.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(Cross2)
{
  env.code_length = 100;

  vita::individual i1(env, true), i2(env, true);

  const unsigned n(1000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
    dist += i1.distance(two_point_crossover(i1, i2));

  const double perc(100.0 * dist /
                    (*env.code_length * env.sset.categories() * n));
  BOOST_CHECK_GT(perc, 45.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_SUITE_END()
