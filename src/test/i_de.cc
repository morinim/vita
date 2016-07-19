/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/i_de.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_i_de
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr double epsilon(0.00001);

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_i_de, F_FACTORY5)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Random creation");

  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_de ind(env);

    BOOST_TEST(ind.debug());
    BOOST_TEST(ind.parameters() == env.sset->categories());
    BOOST_TEST(ind.age() == 0);

    for (unsigned j(0); j < ind.parameters(); ++j)
      BOOST_TEST(std::fabs(ind[j]) <= std::pow(10.0, j + 1));
  }
}

BOOST_AUTO_TEST_CASE(EmptyIndividual)
{
  vita::i_de ind;

  BOOST_TEST(ind.debug());
  BOOST_TEST(ind.empty());
}

BOOST_AUTO_TEST_CASE(Mutation)
{
  vita::i_de t(env);
  const vita::i_de orig(t);

  const unsigned n(1000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation");
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation(0.0, env);
    BOOST_TEST(t == orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  unsigned m(0);

  for (unsigned i(0); i < n; ++i)
  {
    auto i1(orig);

    m += i1.mutation(0.5, env);
  }

  const double perc(100.0 * m / (orig.parameters() * n));
  BOOST_TEST(perc > 47.0);
  BOOST_TEST(perc < 53.0);
}

BOOST_AUTO_TEST_CASE(Comparison, * boost::unit_test::tolerance(0.0001))
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_de a(env);
    BOOST_TEST(a == a);
    BOOST_TEST(distance(a, a) == 0.0);

    vita::i_de b(a);
    BOOST_TEST(a.signature() == b.signature());
    BOOST_TEST(a == b);
    BOOST_TEST(distance(a, b) == 0.0);

    vita::i_de c(env);
    if (a.signature() != c.signature())
    {
      BOOST_TEST(!(a == c));
      BOOST_TEST(distance(a, c) > 0.0);
      BOOST_TEST(distance(a, c) == distance(c, a));
    }
  }
}

BOOST_AUTO_TEST_CASE(Iterators)
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::i_de ind(env);

    unsigned i(0);
    for (const auto &g : ind)
    {
      BOOST_TEST(g.par == ind[i]);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(DeCrossover)
{
  double diff(0), length(0);

  for (unsigned j(0); j < 1000; ++j)
  {
    const vita::i_de p(env);
    vita::i_de a(env), b(env), c(env);

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
    auto off(p.crossover(env.p_cross, env.de.weight, a, a, p));
    BOOST_TEST(off.debug());

    for (unsigned i(0); i < p.parameters(); ++i)
      BOOST_TEST(off[i] == p[i], boost::test_tools::tolerance(epsilon));

    BOOST_TEST_CHECKPOINT("DE self-crossover with mutation");
    off = p.crossover(env.p_cross, env.de.weight, a, b, p);
    BOOST_TEST(off.debug());
    BOOST_TEST(off.age() == std::max({p.age(), a.age(), b.age()}));

    for (unsigned i(0); i < p.parameters(); ++i)
    {
      const auto delta(env.de.weight[1] * std::abs(a[i] - b[i]));

      BOOST_TEST(off[i] > p[i] - delta);
      BOOST_TEST(off[i] < p[i] + delta);

      if (!vita::almost_equal(p[i], off[i]))
        ++diff;
    }

    BOOST_TEST_CHECKPOINT("DE crossover without mutation");
    off = p.crossover(env.p_cross, env.de.weight, a, b, c);
    BOOST_TEST(off.debug());
    BOOST_TEST(off.age() == std::max({p.age(), a.age(), b.age(), c.age()}));
    for (unsigned i(0); i < p.parameters(); ++i)
    {
      const auto delta(env.de.weight[1] * std::abs(a[i] - b[i]));

      if (!vita::almost_equal(p[i], off[i]))
      {
        BOOST_TEST(off[i] > c[i] - delta);
        BOOST_TEST(off[i] < c[i] + delta);
      }
    }

    length += p.parameters();
  }

  BOOST_TEST(diff / length < env.p_cross + 2.0);
  BOOST_TEST(diff / length > env.p_cross - 2.0);
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  BOOST_TEST_CHECKPOINT("Non-empty i_de serialization");
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_de i1(env);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    BOOST_TEST(i1.save(ss));

    vita::i_de i2(env);
    BOOST_TEST(i2.load(ss, env));
    BOOST_TEST(i2.debug());

    BOOST_TEST(i1 == i2);
  }

  BOOST_TEST_CHECKPOINT("Non-empty i_de serialization");
  std::stringstream ss;
  vita::i_de empty;
  BOOST_TEST(empty.save(ss));

  vita::i_de empty1;
  BOOST_TEST(empty1.load(ss, env));
  BOOST_TEST(empty1.debug());
  BOOST_TEST(empty1.empty());

  BOOST_TEST(empty == empty1);

}
BOOST_AUTO_TEST_SUITE_END()
