/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/evolution.h"
#include "kernel/i_mep.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE evolution
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture2.h"
#endif

BOOST_FIXTURE_TEST_SUITE(evolution, F_FACTORY2)

BOOST_AUTO_TEST_CASE(Creation)
{
  using namespace vita;

  for (unsigned n(4); n <= 100; ++n)
    for (auto l(prob.sset.categories() + 2); l <= 100; l += (l < 10 ? 1 : 30))
    {
      prob.env.individuals = n;
      prob.env.code_length = l;
      prob.env.tournament_size = 3;

      const auto eva(std::make_unique<vita::random_evaluator<i_mep>>());

      vita::evolution<i_mep, alps_es> evo1(prob, *eva);
      BOOST_REQUIRE(evo1.debug());

      vita::evolution<i_mep, std_es> evo2(prob, *eva);
      BOOST_REQUIRE(evo2.debug());
    }
}

BOOST_AUTO_TEST_SUITE_END()
