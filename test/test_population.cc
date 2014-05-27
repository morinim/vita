/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/individual/mep.h"
#include "kernel/population.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE population
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_FIXTURE_TEST_SUITE(population, F_FACTORY1)

BOOST_AUTO_TEST_CASE(Creation)
{
  for (unsigned i(0); i < 100; ++i)
  {
    env.individuals = vita::random::between(30u, 200u);
    env.tournament_size = vita::random::between<unsigned>(1, *env.mate_zone);

    vita::population<vita::i_mep> pop(env, sset);

    BOOST_REQUIRE_EQUAL(env.individuals, pop.individuals());

    BOOST_REQUIRE(pop.debug(true));
  }
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 100; ++i)
  {
    env.individuals = vita::random::between(30u, 300u);
    env.tournament_size = vita::random::between<unsigned>(1, *env.mate_zone);

    std::stringstream ss;
    vita::population<vita::i_mep> pop1(env, sset);

    BOOST_REQUIRE(pop1.save(ss));

    vita::population<vita::i_mep> pop2(env, sset);
    BOOST_REQUIRE(pop2.load(ss));
    BOOST_REQUIRE(pop2.debug(true));

    BOOST_REQUIRE_EQUAL(pop1.layers(), pop2.layers());
    BOOST_REQUIRE_EQUAL(pop1.individuals(), pop2.individuals());
    for (unsigned l(0); l < pop1.layers(); ++l)
    {
      BOOST_REQUIRE_EQUAL(pop1.individuals(l), pop2.individuals(l));

      for (unsigned j(0); j < pop1.individuals(); ++j)
      {
        const vita::coord c{l, j};
        BOOST_CHECK_EQUAL(pop1[c], pop2[c]);
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
