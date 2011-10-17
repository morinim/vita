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

#include <cstdlib>
#include <iostream>

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/primitive/sr_pri.h"

#define BOOST_TEST_MODULE Individual
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F()
    : num(new vita::sr::number(-200, 200)),
      f_add(new vita::sr::add()),
      f_sub(new vita::sr::sub()),
      f_mul(new vita::sr::mul()),
      f_ifl(new vita::sr::ifl()),
      f_ife(new vita::sr::ife())
  {
    BOOST_TEST_MESSAGE("Setup fixture");
    env.insert(num);
    env.insert(f_add);
    env.insert(f_sub);
    env.insert(f_mul);
    env.insert(f_ifl);
    env.insert(f_ife);
  }

  ~F()
  {
    BOOST_TEST_MESSAGE("Teardown fixture");
  }

  vita::symbol_ptr num;
  vita::symbol_ptr f_add;
  vita::symbol_ptr f_sub;
  vita::symbol_ptr f_mul;
  vita::symbol_ptr f_ifl;
  vita::symbol_ptr f_ife;

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

    const boost::any v1( (vita::interpreter(i1))() );
    const boost::any v2( (vita::interpreter(i2))() );

    BOOST_REQUIRE(v1.empty() == v2.empty());
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
}

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
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  for (unsigned l(1); l < 100; ++l)
  {
    env.code_length = l;
    vita::individual i(env, true);

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
    BOOST_REQUIRE(a.signature() == b.signature());

    vita::individual c(env, true);
    if (!(a.signature() == c.signature()))
      BOOST_REQUIRE(a != c);
  }
}

BOOST_AUTO_TEST_CASE(Cross0)
{
  env.code_length = 100;

  vita::individual i1(env, true), i2(env, true);

  const unsigned n(1000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
    dist += i1.distance(uniform_crossover(i1, i2));

  const double perc(100.0 * dist / (env.code_length*n));
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
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
  BOOST_CHECK_GT(perc, 48.0);
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

  const double perc(100*dist / (env.code_length*n));
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_SUITE_END()
