/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
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
#include "kernel/ga/search.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

#include "test/fixture6.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("GA")
{

TEST_CASE_FIXTURE(fixture6, "Evolution")
{
  prob.env.individuals = 100;

  vita::log::reporting_level = vita::log::lWARNING;

  auto eva(vita::make_ga_evaluator<vita::i_ga>(
             [](const vita::i_ga &v)
             {
               return std::accumulate(v.begin(), v.end(), 0.0,
                                      [](double sum, auto g)
                                      {
                                        return sum + g;
                                      });
             }));

  vita::evolution<vita::i_ga, vita::alps_es> evo1(prob, eva);
  CHECK(evo1.is_valid());

  const auto s1(evo1.run(1));

  CHECK(s1.best.solution[0] >    8);
  CHECK(s1.best.solution[1] >   95);
  CHECK(s1.best.solution[2] >  950);
  CHECK(s1.best.solution[3] > 9950);

  vita::evolution<vita::i_ga, vita::std_es> evo2(prob, eva);
  CHECK(evo2.is_valid());

  const auto s2(evo2.run(1));

  CHECK(s2.best.solution[0] >    8);
  CHECK(s2.best.solution[1] >   95);
  CHECK(s2.best.solution[2] >  950);
  CHECK(s2.best.solution[3] > 9950);
}

}  // TEST_SUITE("GA")
