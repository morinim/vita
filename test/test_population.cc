/**
 *
 *  \file test_population.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <sstream>

#include "population.h"

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
    env.layers = vita::random::between(1, 7);
    env.individuals = vita::random::between(30, 200);

    const size_t ind_per_layer(env.individuals / env.layers);
    env.tournament_size = vita::random::between<size_t>(
      1, std::min(ind_per_layer, *env.mate_zone));

    vita::population pop(env);

    BOOST_REQUIRE(env.individuals - pop.individuals() < env.layers);

    for (size_t l(0); l < pop.layers(); ++l)
      BOOST_REQUIRE_EQUAL(env.individuals / env.layers, pop.individuals(l));

    BOOST_REQUIRE(pop.debug(true));
  }
}


BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 100; ++i)
  {
    env.layers = vita::random::between(1, 10);
    env.individuals = vita::random::between(30, 300);

    const size_t ind_per_layer(env.individuals / env.layers);
    env.tournament_size = vita::random::between<size_t>(
      1, std::min(ind_per_layer, *env.mate_zone));

    std::stringstream ss;
    vita::population pop1(env);

    BOOST_REQUIRE(pop1.save(ss));

    vita::population pop2(env);
    BOOST_REQUIRE(pop2.load(ss));
    BOOST_REQUIRE(pop2.debug(true));

    BOOST_REQUIRE_EQUAL(pop1.layers(), pop2.layers());
    BOOST_REQUIRE_EQUAL(pop1.individuals(), pop2.individuals());
    for (size_t l(0); l < pop1.layers(); ++l)
      for (size_t i(0); i < pop1.individuals(l); ++i)
      {
        const vita::coord c(l, i);
        BOOST_CHECK_EQUAL(pop1[c], pop2[c]);
      }
  }
}

BOOST_AUTO_TEST_SUITE_END()
