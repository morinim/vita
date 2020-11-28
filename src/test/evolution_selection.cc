/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2020 EOS di Manlio Morini.
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

TEST_SUITE("EVOLUTION SELECTION")
{

TEST_CASE_FIXTURE(fixture2, "Tournament")
{
  using namespace vita;

  prob.env.individuals = 20;
  prob.env.layers      =  1;

  // The test assumes independent draws.
  prob.env.mate_zone   = std::numeric_limits<unsigned>::max();

  population<i_mep> pop(prob);
  summary<i_mep>          sum;
  test_evaluator<i_mep> eva(test_evaluator_type::distinct);

  using coord = typename population<i_mep>::coord;

  selection::tournament<i_mep> sel(pop, eva, sum);

  // Every individual has a unique fitness (`test_evaluator_type::distinct`),
  // so there is just one maximum-fitness-individual.
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

TEST_CASE_FIXTURE(fixture2, "ALPS")
{
  using namespace vita;

  const auto alps_select =
    [this](double psl)
    {
      prob.env.individuals       =  20;
      prob.env.layers            =   1;
      prob.env.tournament_size   =   2;
      prob.env.alps.p_same_layer = psl;

      population<i_mep> pop(prob);
      summary<i_mep>          sum;
      test_evaluator<i_mep>   eva;

      pop.add_layer();

      std::vector<unsigned> layer_count(pop.layers());

      using coord = typename population<i_mep>::coord;

      const unsigned n(2000);
      for (unsigned i(0); i < n; ++i)
      {
        selection::alps<i_mep> sel(pop, eva, sum);

        const auto parents(sel.run());
        CHECK(parents.size() == 2);

        const bool is_sorted(
          std::is_sorted(parents.begin(), parents.end(),
                         [&](const coord &c1, const coord &c2)
                         {
                           std::pair<bool, fitness_t> v1;
                           v1.first = !vita::alps::aged(pop, c1);
                           v1.second = eva(pop[c1]);

                           std::pair<bool, fitness_t> v2;
                           v2.first = !vita::alps::aged(pop, c2);
                           v2.second = eva(pop[c2]);

                           return v1 > v2;
                         }));
        CHECK(is_sorted);

        ++layer_count[parents[0].layer];
        ++layer_count[parents[1].layer];
      }

      return layer_count;
    };

  const auto lc1(alps_select(1.0));
  auto half((lc1[0] + lc1[1]) / 2.0);
  CHECK(lc1[0] >= half * 0.95);
  CHECK(lc1[0] <= half * 1.05);

  const auto lc2(alps_select(0.75));
  CHECK(lc2[0] > half);
  CHECK(lc2[1] < half);

  const auto lc3(alps_select(0.50));
  CHECK(lc3[0] > lc2[0]);
  CHECK(lc3[1] < lc2[1]);
}

}  // TEST_SUITE("EVOLUTION SELECTION")
