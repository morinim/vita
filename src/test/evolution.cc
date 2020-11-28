/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/evolution.h"
#include "kernel/gp/mep/i_mep.h"

#include "test/fixture2.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("EVOLUTION")
{

TEST_CASE_FIXTURE(fixture2, "Creation")
{
  using namespace vita;

  for (unsigned n(4); n <= 100; ++n)
    for (auto l(prob.sset.categories() + 2); l <= 100; l += (l < 10 ? 1 : 30))
    {
      prob.env.individuals = n;
      prob.env.mep.code_length = l;
      prob.env.tournament_size = 3;

      const auto eva(std::make_unique<vita::test_evaluator<i_mep>>());

      vita::evolution<i_mep, alps_es> evo1(prob, *eva);
      CHECK(evo1.is_valid());

      vita::evolution<i_mep, std_es> evo2(prob, *eva);
      CHECK(evo2.is_valid());
    }
}

}  // TEST_SUITE("EVOLUTION")
