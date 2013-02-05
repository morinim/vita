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
  env.code_length = 100;

  vita::population pop(env);

  BOOST_REQUIRE_EQUAL(*env.code_length, pop.size());

  for (size_t i(0); i < pop.size(); ++i)
    BOOST_CHECK(pop[i].debug());
}


BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 100; ++i)
  {
    std::stringstream ss;
    vita::population pop1(env);

    BOOST_REQUIRE(pop1.save(ss));

    vita::population pop2(env);
    BOOST_REQUIRE(pop2.load(ss));
    BOOST_REQUIRE(pop2.debug());

    BOOST_REQUIRE_EQUAL(pop1.size(), pop2.size());
    for (size_t i(0); i < pop1.size(); ++i)
      BOOST_CHECK_EQUAL(pop1[i], pop2[i]);
  }
}

BOOST_AUTO_TEST_SUITE_END()
