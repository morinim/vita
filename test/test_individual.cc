/**
 *
 *  \file test_individual.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma GCC diagnostic ignored "-Wtype-limits"

#include <cstdlib>
#include <iostream>

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"
#include "kernel/primitive/factory.h"

#define BOOST_TEST_MODULE Individual
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F()
  {
    BOOST_TEST_MESSAGE("Setup fixture");

    vita::symbol_factory &factory(vita::symbol_factory::instance());

    env.insert(factory.make("NUMBER", vita::d_double, {0}));
    env.insert(factory.make("ADD", vita::d_double, {0}));
    env.insert(factory.make("SUB", vita::d_double, {0}));
    env.insert(factory.make("MUL", vita::d_double, {0}));
    env.insert(factory.make("IFL", vita::d_double, {0}));
    env.insert(factory.make("IFE", vita::d_double, {0}));

    env.insert(factory.make("apple", vita::d_string, {1}));
    env.insert(factory.make("pear", vita::d_string, {1}));
    env.insert(factory.make("grapefruit", vita::d_string, {1}));
    env.insert(factory.make("IFE", vita::d_string, {0, 1}));
    env.insert(factory.make("LENGTH", vita::d_string, {1, 0}));
  }

  ~F()
  {
    BOOST_TEST_MESSAGE("Teardown fixture");
  }

  vita::environment env;
};

BOOST_FIXTURE_TEST_SUITE(Individual, F)

BOOST_AUTO_TEST_CASE(Compact)
{
  env.code_length = 100;

  BOOST_TEST_CHECKPOINT("Functional equivalence.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::individual i1(env, true);
    const vita::individual i2(i1.compact());

    std::cout << i1 << std::endl << i2 << std::endl;

    const boost::any v1( (vita::interpreter(i1))() );
    const boost::any v2( (vita::interpreter(i2))() );

    BOOST_REQUIRE_EQUAL(v1.empty(), v2.empty());
    if (!v1.empty() && !v2.empty())
      BOOST_REQUIRE_EQUAL(boost::any_cast<double>(v1),
                          boost::any_cast<double>(v2));
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
/*
BOOST_AUTO_TEST_CASE(Mutation)
{
  env.code_length = 100;

  vita::individual ind(env, true);
  const vita::individual orig(ind);

  env.p_mutation = 0;

  BOOST_TEST_CHECKPOINT("Zero probability mutation.");
  for (unsigned i(0); i < 1000; ++i)
  {
    ind = ind.mutation();
    BOOST_REQUIRE_EQUAL(ind, orig);
  }

  env.p_mutation = 0.5;
  double dist(0.0);
  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  const unsigned n(1000);
  for (unsigned i(0); i < n; ++i)
  {
    const vita::individual i1(ind);

    ind = ind.mutation();
    dist += i1.distance(ind);
  }

  const double perc(100*dist / (env.code_length*n));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}
*/
BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation.");
  for (unsigned l(env.sset.categories() + 1); l < 100; ++l)
  {
    env.code_length = l;
    vita::individual i(env, true);
    // std::cout << i << std::endl;

    BOOST_REQUIRE(i.check());
    BOOST_REQUIRE_EQUAL(i.size(), l);
  }

  BOOST_TEST_CHECKPOINT("Sticky symbol must exits!");
  const vita::symbol_ptr n_123(vita::symbol_factory::instance().make(
                                 "123", vita::d_double, {}));
  env.insert(n_123, true);
  for (unsigned l(env.sset.categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::individual i(env, true);

    BOOST_REQUIRE_EQUAL(i[l-1].sym, n_123);
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
/*
BOOST_AUTO_TEST_CASE(Cross0)
{
  env.code_length = 100;

  vita::individual i1(env, true), i2(env, true);

  const unsigned n(1000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
    dist += i1.distance(uniform_crossover(i1, i2));

  const double perc(100.0 * dist / (env.code_length*n));
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

  const double perc(100.0 * dist / (env.code_length*n));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}

BOOST_AUTO_TEST_CASE(Cross2)
{
  env.code_length = 100;

  vita::individual i1(env, true), i2(env, true);

  const unsigned n(1000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
    dist += i1.distance(two_point_crossover(i1, i2));

  const double perc(100*dist / (env.code_length*n));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}
*/
BOOST_AUTO_TEST_SUITE_END()
