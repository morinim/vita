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
#include "kernel/population.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_population
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_population, F_FACTORY1)

BOOST_AUTO_TEST_CASE(creation)
{
  env.layers = 1;

  for (unsigned i(0); i < 100; ++i)
  {
    env.individuals = vita::random::between(30u, 200u);

    vita::population<vita::i_mep> pop(env);

    BOOST_TEST(env.individuals == pop.individuals());
    BOOST_TEST(pop.debug());
  }
}

BOOST_AUTO_TEST_CASE(layers_and_indeividuals)
{
  for (unsigned i(0); i < 100; ++i)
  {
    env.individuals = vita::random::between(30u, 200u);
    env.layers = vita::random::between(1u, 10u);

    vita::population<vita::i_mep> pop(env);

    for (unsigned l(0); l < pop.layers(); ++l)
    {
      const auto n(vita::random::between(0u, pop.individuals(l)));

      const auto before(pop.individuals(l));

      for (unsigned j(0); j < n; ++j)
        pop.pop_from_layer(l);

      BOOST_TEST(pop.individuals(l) == before - n);
    }

    unsigned count(std::accumulate(pop.begin(), pop.end(), 0,
                                   [](auto acc, auto) { return ++acc; }));

    BOOST_TEST(count == pop.individuals());
  }
}

BOOST_AUTO_TEST_CASE(serialization)
{
  using namespace vita;

  for (unsigned i(0); i < 100; ++i)
  {
    env.individuals = random::between(30u, 300u);
    env.tournament_size = random::between<unsigned>(1, env.mate_zone);

    std::stringstream ss;
    vita::population<i_mep> pop1(env);

    BOOST_TEST(pop1.save(ss));

    decltype(pop1) pop2(env);
    BOOST_TEST(pop2.load(ss, env));
    BOOST_TEST(pop2.debug());

    BOOST_TEST(pop1.layers() == pop2.layers());
    BOOST_TEST(pop1.individuals() == pop2.individuals());
    for (unsigned l(0); l < pop1.layers(); ++l)
    {
      BOOST_TEST(pop1.individuals(l) == pop2.individuals(l));

      for (unsigned j(0); j < pop1.individuals(); ++j)
      {
        const vita::population<i_mep>::coord c{l, j};
        BOOST_TEST(pop1[c] == pop2[c]);
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
