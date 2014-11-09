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

#include "kernel/ga/i_ga.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_i_ga
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr double epsilon(0.00001);

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_i_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Random creation");

  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_ga ind(env, sset);
    vita::interpreter<vita::i_ga> check(&ind);

    BOOST_REQUIRE(ind.debug());
    BOOST_REQUIRE_EQUAL(ind.parameters(), sset.categories());
    BOOST_REQUIRE_EQUAL(ind.age(), 0);
    BOOST_REQUIRE_EQUAL(check.penalty(), 0);
  }
}

BOOST_AUTO_TEST_CASE(Penalty)
{
  for (unsigned i(0); i < 100; ++i)
  {
    vita::i_ga ind(env, sset);
    vita::interpreter<vita::i_ga> check(&ind);

    BOOST_REQUIRE_EQUAL(check.penalty(), 0);
    ind[0] = -20.0;
    BOOST_REQUIRE_EQUAL(check.penalty(), 1);
    ind[0] = 20.0;
    BOOST_REQUIRE_EQUAL(check.penalty(), 1);
    ind[1] = -200.0;
    BOOST_REQUIRE_EQUAL(check.penalty(), 2);
    ind[1] = 200.0;
    BOOST_REQUIRE_EQUAL(check.penalty(), 2);
    ind[2] = -2000.0;
    BOOST_REQUIRE_EQUAL(check.penalty(), 3);
    ind[2] = 2000.0;
    BOOST_REQUIRE_EQUAL(check.penalty(), 3);
  }
}

BOOST_AUTO_TEST_CASE(Mutation)
{
  vita::i_ga t(env, sset);
  const vita::i_ga orig(t);

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

  const double perc(100.0 * double(diff) / double(orig.parameters() * n));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 53.0);
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_ga a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);
    BOOST_REQUIRE_EQUAL(a.distance(a), 0);

    vita::i_ga b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());
    BOOST_REQUIRE_EQUAL(a, b);
    BOOST_REQUIRE_EQUAL(a.distance(b), 0);

    vita::i_ga c(env, sset);
    if (a.signature() != c.signature())
    {
      BOOST_REQUIRE_NE(a, c);
      BOOST_REQUIRE_GT(a.distance(c), 0);
      BOOST_REQUIRE_EQUAL(a.distance(c), c.distance(a));
    }
  }
}

BOOST_AUTO_TEST_CASE(Iterators)
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::i_ga ind(env, sset);

    unsigned i(0);
    for (const auto &l : ind)
    {
      BOOST_CHECK_EQUAL(ind[l].par, ind[i]);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(StandardCrossover)
{
  vita::i_ga i1(env, sset), i2(env, sset);

  double dist(0.0);
  const unsigned n(1000);
  for (unsigned j(0); j < n; ++j)
  {
    if (vita::random::boolean())
      i1.inc_age();
    if (vita::random::boolean())
      i2.inc_age();

    const auto ic(i1.crossover(i2));
    BOOST_CHECK(ic.debug(true));
    BOOST_REQUIRE_EQUAL(ic.age(), std::max(i1.age(), i2.age()));

    dist += i1.distance(ic);
  }

  // +1 since we have at least one gene involved in crossover.
  const double perc(100.0 * dist / ((sset.categories() + 1) * n));
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(DeCrossover)
{
  double diff(0), length(0);

  for (unsigned j(0); j < 1000; ++j)
  {
    const vita::i_ga p(env, sset);
    vita::i_ga a(env, sset), b(env, sset), c(env, sset);

    const auto n_a(vita::random::between<unsigned>(0, 100));
    for (unsigned k(0); k < n_a; ++k)
      a.inc_age();
    const auto n_b(vita::random::between<unsigned>(0, 100));
    for (unsigned k(0); k < n_b; ++k)
      b.inc_age();
    const auto n_c(vita::random::between<unsigned>(0, 100));
    for (unsigned k(0); k < n_c; ++k)
      c.inc_age();

    BOOST_TEST_CHECKPOINT("DE self-crossover without mutation");
    auto off(p.crossover(a, a, p));
    BOOST_CHECK(off.debug(true));

    for (unsigned i(0); i < p.parameters(); ++i)
      BOOST_CHECK_CLOSE(off[i], p[i], epsilon);

    BOOST_TEST_CHECKPOINT("DE self-crossover with mutation");
    off = p.crossover(a, b, p);
    BOOST_CHECK(off.debug(true));
    BOOST_REQUIRE_EQUAL(off.age(), std::max({p.age(), a.age(), b.age()}));

    for (unsigned i(0); i < p.parameters(); ++i)
    {
      const auto delta(env.de.weight[1] * std::abs(a[i] - b[i]));

      BOOST_CHECK_GT(off[i], p[i] - delta);
      BOOST_CHECK_LT(off[i], p[i] + delta);

      if (!vita::almost_equal(p[i], off[i]))
        ++diff;
    }

    BOOST_TEST_CHECKPOINT("DE crossover without mutation");
    off = p.crossover(a, b, c);
    BOOST_CHECK(off.debug(true));
    BOOST_REQUIRE_EQUAL(off.age(), std::max({p.age(), a.age(), b.age(),
                                             c.age()}));
    for (unsigned i(0); i < p.parameters(); ++i)
    {
      const auto delta(env.de.weight[1] * std::abs(a[i] - b[i]));

      if (!vita::almost_equal(p[i], off[i]))
      {
        BOOST_CHECK_GT(off[i], c[i] - delta);
        BOOST_CHECK_LT(off[i], c[i] + delta);
      }
    }

    length += p.parameters();
  }

  BOOST_CHECK_LT(diff / length, env.p_cross + 2.0);
  BOOST_CHECK_GT(diff / length, env.p_cross - 2.0);
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_ga i1(env, sset);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    BOOST_REQUIRE(i1.save(ss));

    vita::i_ga i2(env, sset);
    BOOST_REQUIRE(i2.load(ss));
    BOOST_REQUIRE(i2.debug());

    BOOST_CHECK_EQUAL(i1, i2);
  }
}
BOOST_AUTO_TEST_SUITE_END()
