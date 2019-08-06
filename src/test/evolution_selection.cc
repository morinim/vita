/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2019 EOS di Manlio Morini.
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

#include "test/fixture2.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("EVOLUTION SELECTION")
{

TEST_CASE_FIXTURE(fixture2, "Tournament")
{
  using namespace vita;

  prob.env.individuals = 20;
  prob.env.layers      =  1;

  // The test assumes independent draws.
  prob.env.mate_zone   = std::numeric_limits<unsigned>::max();

  population<i_mep>    pop(prob);
  test_evaluator<i_mep>      eva;
  summary<i_mep>             sum;

  using coord = typename population<i_mep>::coord;

  selection::tournament<i_mep> sel(pop, eva, sum);

  for (unsigned ts(1); ts < prob.env.individuals; ++ts)
  {
    prob.env.tournament_size = ts;

    coord max{0, 0};
    for (unsigned i(1); i < pop.individuals(); ++i)
      if (eva(pop[{0, i}]) > eva(pop[max]))
        max = coord{0, i};

    double p_not_present((pop.individuals() - 1) /
                         static_cast<double>(pop.individuals()));
    double p_present(1.0 - std::pow(p_not_present, ts));

    const unsigned n(2000);
    unsigned found(0);
    for (unsigned i(0); i < n; ++i)
    {
      auto parents(sel.run());

      CHECK(parents.size() == prob.env.tournament_size);

      const bool is_sorted(
        std::is_sorted(parents.begin(), parents.end(),
                       [&](const coord &c1, const coord &c2)
                       {
                         return eva(pop[c1]) > eva(pop[c2]);
                       }) );

      CHECK(is_sorted);

      if (std::find(parents.begin(), parents.end(), max) != parents.end())
        ++found;
    }
    const double frequency(static_cast<double>(found) / n);

    CHECK(frequency > p_present - 0.1);
    CHECK(frequency < p_present + 0.1);
  }
}

}  // TEST_SUITE("EVOLUTION SELECTION")
