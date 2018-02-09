/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/evaluator.h"
#include "kernel/ga/i_ga.h"
#include "kernel/ga/interpreter.h"
#include "kernel/ga/search.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_ga
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(Evolution)
{
  prob.env.individuals = 100;

  vita::log::reporting_level = vita::log::WARNING;

  auto eva(vita::make_ga_evaluator<vita::i_ga>(
             [](const vita::i_ga &v)
             { return std::accumulate(v.begin(), v.end(), 0.0,
                                      [](double sum, const vita::gene &g)
                                      {
                                        return sum + g.par;
                                      });
             }));

  vita::evolution<vita::i_ga, vita::alps_es> evo1(prob, eva);
  BOOST_REQUIRE(evo1.debug());

  const auto s1(evo1.run(1));

  BOOST_CHECK_GT(s1.best.solution[0].par, 8.0);
  BOOST_CHECK_GT(s1.best.solution[1].par, 95.0);
  BOOST_CHECK_GT(s1.best.solution[2].par, 950.0);
  BOOST_CHECK_GT(s1.best.solution[3].par, 9950.0);

  vita::evolution<vita::i_ga, vita::std_es> evo2(prob, eva);
  BOOST_REQUIRE(evo2.debug());

  const auto s2(evo2.run(1));

  BOOST_CHECK_GT(s2.best.solution[0].par, 8.0);
  BOOST_CHECK_GT(s2.best.solution[1].par, 95.0);
  BOOST_CHECK_GT(s2.best.solution[2].par, 950.0);
  BOOST_CHECK_GT(s2.best.solution[3].par, 9950.0);
}
BOOST_AUTO_TEST_SUITE_END()
