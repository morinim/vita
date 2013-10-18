/**
 *
 *  \file test_evolution.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>

#include "kernel/evolution.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE evolution
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture2.h"
#endif

BOOST_FIXTURE_TEST_SUITE(evolution, F_FACTORY2)

BOOST_AUTO_TEST_CASE(Creation)
{
  for (unsigned n(4); n <= 100; ++n)
    for (unsigned l(sset.categories() + 2); l <= 100; l+= (l < 10 ? 1 : 30))
    {
      env.individuals = n;
      env.code_length = l;
      env.tournament_size = 3;

      const std::unique_ptr<vita::evaluator> eva(
        vita::make_unique<vita::random_evaluator>());

      vita::evolution<vita::alps_es> evo1(env, sset, *eva.get());
      BOOST_REQUIRE(evo1.debug(true));

      vita::evolution<vita::std_es> evo2(env, sset, *eva.get());
      BOOST_REQUIRE(evo2.debug(true));
    }
}

BOOST_AUTO_TEST_SUITE_END()
