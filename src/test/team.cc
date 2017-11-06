/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
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

BOOST_FIXTURE_TEST_SUITE(t_team, F_FACTORY1)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation");
  for (unsigned l(prob.sset.categories() + 2); l < 100; ++l)
  {
    prob.env.mep.code_length = l;
    vita::team<vita::i_mep> t(prob);

    BOOST_TEST(t.debug());
    BOOST_TEST(t.age() == 0);
  }
}

BOOST_AUTO_TEST_CASE(Mutation)
{
  prob.env.mep.code_length = 100;

  vita::team<vita::i_mep> t(prob);
  const auto orig(t);

  BOOST_TEST(t.individuals() > 0);

  const unsigned n(4000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation");
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation(0.0, prob);
    BOOST_TEST(t == orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");

  double diff(0.0), length(0.0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::team<vita::i_mep> t1{t};

    t.mutation(0.5, prob);
    diff += distance(t, t1);
    length += t1.active_symbols();
  }

  const double perc(100.0 * diff / length);
  BOOST_TEST(perc > 47.0);
  BOOST_TEST(perc < 52.0);
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::team<vita::i_mep> a(prob);
    BOOST_TEST(a == a);
    BOOST_TEST(distance(a, a) == 0);

    vita::team<vita::i_mep> b(a);
    BOOST_TEST(a.signature() == b.signature());
    BOOST_TEST(a == b);
    BOOST_TEST(distance(a, b) == 0);

    vita::team<vita::i_mep> c(prob);
    if (a.signature() != c.signature())
    {
      BOOST_TEST(a != c);
      BOOST_TEST(distance(a, c) > 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(Iterators)
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::team<vita::i_mep> t(prob);

    unsigned i(0);
    for (const auto &ind : t)
    {
      BOOST_TEST(ind == t[i]);
      ++i;
    }
  }
}

BOOST_AUTO_TEST_CASE(t_crossover)
{
  using namespace vita;

  prob.env.mep.code_length = 100;

  team<vita::i_mep> t1(prob), t2(prob);

  const unsigned n(2000);
  for (unsigned j(0); j < n; ++j)
  {
    const auto tc(crossover(t1, t2));
    BOOST_TEST(tc.debug());

    for (unsigned x(0); x < tc.individuals(); ++x)
      for (index_t i(0); i != tc[x].size(); ++i)
        for (category_t c(0); c < tc[x].categories(); ++c)
        {
          const locus l{i, c};

          BOOST_TEST((tc[x][l] == t1[x][l] || tc[x][l] == t2[x][l]));
        }
  }
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::team<vita::i_mep> t1(prob);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      t1.inc_age();

    BOOST_TEST(t1.save(ss));

    vita::team<vita::i_mep> t2(prob);
    BOOST_TEST(t2.load(ss, prob));
    BOOST_TEST(t2.debug());

    BOOST_TEST(t1 == t2);
  }
}
BOOST_AUTO_TEST_SUITE_END()
