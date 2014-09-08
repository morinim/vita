/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/i_ga.h"
#include "kernel/ga/ga_evaluator.h"
#include "kernel/ga/ga_search.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_ga
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_ga, F_FACTORY5)

BOOST_AUTO_TEST_CASE(Evaluator)
{
  auto f = [](const std::vector<double> &v)
           { return std::accumulate(v.begin(), v.end(), 0.0); };

  auto eva(vita::make_evaluator<vita::i_ga>(f));

  vita::fitness_t eva_prev;
  double v_prev;

  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_ga ind(env, sset);
    BOOST_REQUIRE(ind.debug());

    const auto eva_ret(eva(ind));
    BOOST_REQUIRE_LE(eva_ret, 0.0);

    const auto v(f(ind));

    if (i)
    {
      if (v_prev < v)
        BOOST_REQUIRE_LT(eva_prev, eva_ret);
      else
        BOOST_REQUIRE_GE(eva_prev, eva_ret);
    }

    v_prev = v;
    eva_prev = eva_ret;
  }
}

BOOST_AUTO_TEST_CASE(Evolution)
{
  env.individuals = 100;
  env.verbosity = 0;

  auto eva(vita::make_evaluator<vita::i_ga>(
    [](const std::vector<double> &v)
    { return std::accumulate(v.begin(), v.end(), 0.0); }));

  vita::evolution<vita::i_ga, vita::alps_es> evo1(env, sset, eva);
  BOOST_REQUIRE(evo1.debug(true));

  const auto s1(evo1.run(1));

  BOOST_CHECK_GT(s1.best->ind[0], 8.0);
  BOOST_CHECK_GT(s1.best->ind[1], 95.0);
  BOOST_CHECK_GT(s1.best->ind[2], 990.0);
  BOOST_CHECK_GT(s1.best->ind[3], 9980.0);

  vita::evolution<vita::i_ga, vita::std_es> evo2(env, sset, eva);
  BOOST_REQUIRE(evo2.debug(true));

  const auto s2(evo2.run(1));

  BOOST_CHECK_GT(s2.best->ind[0], 8.0);
  BOOST_CHECK_GT(s2.best->ind[1], 95.0);
  BOOST_CHECK_GT(s2.best->ind[2], 980.0);
  BOOST_CHECK_GT(s2.best->ind[3], 9980.0);
}

BOOST_AUTO_TEST_CASE(Search)
{
  env.individuals = 100;
  //env.verbosity = 0;

  vita::problem prob;
  prob.env = env;
  prob.sset = std::move(sset);

  vita::ga_search<vita::i_ga, vita::de_es> s(prob);
  BOOST_REQUIRE(s.debug(true));

  s.set_evaluator(vita::make_unique_evaluator<vita::i_ga>(
                    [](const std::vector<double> &v)
                    { return std::accumulate(v.begin(), v.end(), 0.0); }));

  const auto res(s.run());

  BOOST_CHECK_GT(res[0], 8.0);
  BOOST_CHECK_GT(res[1], 95.0);
  BOOST_CHECK_GT(res[2], 990.0);
  BOOST_CHECK_GT(res[3], 9980.0);
}

BOOST_AUTO_TEST_SUITE_END()
