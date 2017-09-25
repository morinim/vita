/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017 EOS di Manlio Morini.
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
#define BOOST_TEST_MODULE t_evolution_selection
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture2.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_evolution_selection, F_FACTORY2)

BOOST_AUTO_TEST_CASE(t_tournament)
{
  using namespace vita;

  prob.env.individuals = 10;
  prob.env.layers      =  1;

  population<i_mep>    pop(prob);
  test_evaluator<i_mep>      eva;
  summary<i_mep>             sum;

  using coord = typename population<i_mep>::coord;

  selection::tournament<i_mep> sel(pop, eva, sum);

  for (unsigned ts(1); ts < 10; ++ts)
  {
    prob.env.tournament_size = ts;

    coord max{0, 0};
    for (unsigned i(1); i < pop.individuals(); ++i)
      if (eva(pop[{0, i}]) > eva(pop[max]))
        max = coord{0, i};

    double p_not_present((pop.individuals() - 1) /
                         static_cast<double>(pop.individuals()));
    double p_present(1.0 - std::pow(p_not_present, ts));

    const unsigned n(1000);
    unsigned found(0);
    for (unsigned i(0); i < n; ++i)
    {
      auto parents(sel.run());

      BOOST_TEST(parents.size() == prob.env.tournament_size);

      const bool is_sorted(
        std::is_sorted(parents.begin(), parents.end(),
                       [&](const coord &c1, const coord &c2)
                       {
                         return eva(pop[c1]) > eva(pop[c2]);
                       }) );

      BOOST_CHECK(is_sorted);

      if (std::find(parents.begin(), parents.end(), max) != parents.end())
        ++found;
    }
    const double frequency(static_cast<double>(found) / n);

    //std::cout << p_present << "    " << frequency << "  " << std::endl;

    BOOST_CHECK_GT(frequency, p_present - 0.1);
    BOOST_CHECK_LT(frequency, p_present + 0.1);
  }
}
/*
BOOST_AUTO_TEST_CASE(t_fuss)
{
  using namespace vita;

  prob.env.individuals = 10;
  prob.env.layers      =  1;

  population<i_mep     pop(prob);
  test_evaluator<i_mep>      eva;
  summary<i_mep>             sum;

  selection::fuss<i_mep> sel(pop, eva, sum);

  distribution<fitness_t> dist;

  std::map<fitness_t, unsigned> counter;

  BOOST_TEST_CHECKPOINT("FUSS Uniform fitness distribution");

  for (unsigned ts(5); ts < 10; ++ts)
  {
    prob.env.tournament_size = ts;

    counter.clear();

    for (unsigned i(0); i < pop.individuals(0); ++i)
      sum.az.add(pop[{0, i}], eva(pop[{0, i}]), 0);

    const unsigned n(2000);
    for (unsigned i(0); i < n; ++i)
    {
      auto parents(sel.run());

      BOOST_CHECK_EQUAL(parents.size(), prob.env.tournament_size);

      dist.add(eva(pop[parents[0]]));

      ++counter[eva(pop[parents[0]])];
    }

    const auto range(dist.max()[0] - dist.min()[0]);
    BOOST_CHECK_GT(dist.mean()[0], range / 2 * 0.85);
    BOOST_CHECK_LT(dist.mean()[0], range / 2 * 1.15);

    // For a uniform distribution:
    // * mean = range /2
    // * variance = range^2 / 12
    BOOST_CHECK_GT(dist.variance()[0], range*range / 12 * 0.85);
    BOOST_CHECK_LT(dist.variance()[0], range*range / 12 * 1.15);

    const double avg(static_cast<double>(n) / pop.individuals(0));
    for (const auto &e : counter)
    {
      BOOST_CHECK_GT(e.second, avg * 0.80);
      BOOST_CHECK_LT(e.second, avg * 1.20);
    }
  }

  std::cout << std::endl;
  for (const auto &v : counter)
    std::cout << "Fitness: " << v.first << "  Frequency: " << v.second
              << std::endl;
  std::cout << std::endl;
}
*/
BOOST_AUTO_TEST_SUITE_END()
