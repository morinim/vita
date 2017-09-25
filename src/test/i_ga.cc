/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017 EOS di Manlio Morini.
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

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_i_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Random creation");

  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_ga ind(prob);
    vita::interpreter<vita::i_ga> check(&ind);

    BOOST_TEST(ind.debug());
    BOOST_TEST(ind.parameters() == prob.sset.categories());
    BOOST_TEST(ind.age() == 0);
    BOOST_TEST(check.penalty() == 0);
  }
}

BOOST_AUTO_TEST_CASE(EmptyIndividual)
{
  vita::i_ga ind;

  BOOST_TEST(ind.debug());
  BOOST_TEST(ind.empty());
}


BOOST_AUTO_TEST_CASE(Penalty, * boost::unit_test::tolerance(0.000001))
{
  for (unsigned i(0); i < 100; ++i)
  {
    vita::i_ga ind(prob);
    vita::interpreter<vita::i_ga> check(&ind);

    BOOST_TEST(check.penalty() == 0.0);

    ind[0].par = 20.0;
    const auto p1(check.penalty());
    BOOST_TEST(p1 > 0.0);

    ind[0].par = -20.0;
    const auto p1n(check.penalty());
    BOOST_TEST(p1 == p1n);

    ind[1].par = 200.0;
    const auto p2(check.penalty());
    BOOST_TEST(p2 > p1);

    ind[1].par = -200.0;
    const auto p2n(check.penalty());
    BOOST_TEST(p2 > p1);
    BOOST_TEST(p2 == p2n);

    ind[2].par = 2000.0;
    const auto p3(check.penalty());
    BOOST_TEST(p3 > p2);

    ind[2].par = -2000.0;
    const auto p3n(check.penalty());
    BOOST_TEST(p3n > p2);
    BOOST_TEST(p3 == p3n);
  }
}

BOOST_AUTO_TEST_CASE(Mutation)
{
  vita::i_ga t(prob);
  const vita::i_ga orig(t);

  const unsigned n(1000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation");
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation(0.0, prob);
    BOOST_TEST(t == orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  unsigned diff(0);

  for (unsigned i(0); i < n; ++i)
  {
    auto i1(orig);

    i1.mutation(0.5, prob);
    diff += orig.distance(i1);
  }

  const double perc(100.0 * double(diff) / double(orig.parameters() * n));
  BOOST_TEST(perc > 47.0);
  BOOST_TEST(perc < 53.0);
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_ga a(prob);
    BOOST_TEST(a == a);
    BOOST_TEST(a.distance(a) == 0);

    vita::i_ga b(a);
    BOOST_TEST(a.signature() == b.signature());
    BOOST_TEST(a == b);
    BOOST_TEST(a.distance(b) == 0);

    vita::i_ga c(prob);
    if (a.signature() != c.signature())
    {
      BOOST_TEST(!(a == c));
      BOOST_TEST(a.distance(c) > 0);
      BOOST_TEST(a.distance(c) == c.distance(a));
    }
  }
}

BOOST_AUTO_TEST_CASE(Iterators)
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::i_ga ind(prob);

    unsigned i(0);
    for (const auto &g : ind)
    {
      BOOST_TEST(g == ind[i]);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(StandardCrossover)
{
  vita::i_ga i1(prob), i2(prob);

  double dist(0.0);
  const unsigned n(1000);
  for (unsigned j(0); j < n; ++j)
  {
    if (vita::random::boolean())
      i1.inc_age();
    if (vita::random::boolean())
      i2.inc_age();

    const auto ic(crossover(i1, i2));
    BOOST_CHECK(ic.debug());
    BOOST_REQUIRE_EQUAL(ic.age(), std::max(i1.age(), i2.age()));

    dist += i1.distance(ic);
  }

  // +1 since we have at least one gene involved in crossover.
  const double perc(100.0 * dist / ((prob.sset.categories() + 1) * n));
  BOOST_CHECK_GT(perc, 48.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  BOOST_TEST_CHECKPOINT("Non-empty i_ga serialization");
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_ga i1(prob);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    BOOST_TEST(i1.save(ss));

    vita::i_ga i2(prob);
    BOOST_TEST(i2.load(ss, prob));
    BOOST_TEST(i2.debug());

    BOOST_CHECK_EQUAL(i1, i2);
  }

  BOOST_TEST_CHECKPOINT("Non-empty i_ga serialization");
  std::stringstream ss;
  vita::i_ga empty;
  BOOST_TEST(empty.save(ss));

  vita::i_ga empty1;
  BOOST_TEST(empty1.load(ss, prob));
  BOOST_TEST(empty1.debug());
  BOOST_TEST(empty1.empty());

  BOOST_REQUIRE_EQUAL(empty, empty1);

}
BOOST_AUTO_TEST_SUITE_END()
