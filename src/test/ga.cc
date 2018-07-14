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

#include "test/fixture5.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("GA")
{

TEST_CASE_FIXTURE(fixture5, "Evolution")
{
  prob.env.individuals = 100;

  vita::log::reporting_level = vita::log::lWARNING;

  auto eva(vita::make_ga_evaluator<vita::i_ga>(
             [](const vita::i_ga &v)
             {
               return std::accumulate(v.begin(), v.end(), 0.0,
                                      [](double sum, const vita::gene &g)
                                      {
                                        return sum + g.par;
                                      });
             }));

  vita::evolution<vita::i_ga, vita::alps_es> evo1(prob, eva);
  CHECK(evo1.debug());

  const auto s1(evo1.run(1));

  CHECK(s1.best.solution[0].par >    8.0);
  CHECK(s1.best.solution[1].par >   95.0);
  CHECK(s1.best.solution[2].par >  950.0);
  CHECK(s1.best.solution[3].par > 9950.0);

  vita::evolution<vita::i_ga, vita::std_es> evo2(prob, eva);
  CHECK(evo2.debug());

  const auto s2(evo2.run(1));

  CHECK(s2.best.solution[0].par >    8.0);
  CHECK(s2.best.solution[1].par >   95.0);
  CHECK(s2.best.solution[2].par >  950.0);
  CHECK(s2.best.solution[3].par > 9950.0);
}

}  // TEST_SUITE("GA")
