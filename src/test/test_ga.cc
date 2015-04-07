/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
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

BOOST_FIXTURE_TEST_SUITE(t_ga1, F_FACTORY5)

BOOST_AUTO_TEST_CASE(Penalty)
{
  vita::i_ga ind(env, sset);
  BOOST_REQUIRE(ind.debug());
/*
  vita::interpreter<vita::i_ga> intr(&ind);

  BOOST_REQUIRE_EQUAL(intr.penalty(), 0);

  ind[0] = 20;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 1);

  ind[1] = 200;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 2);

  ind[2] = 2000;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 3);

  ind[3] = 20000;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 4);*/
}

BOOST_AUTO_TEST_CASE(Evaluator)
{
  auto f = [](const std::vector<double> &v)
           { return std::accumulate(v.begin(), v.end(), 0.0); };

  auto eva(vita::make_ga_evaluator<vita::i_ga>(f));

  vita::fitness_t eva_prev;
  double v_prev(0.0);

  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_ga ind(env, sset);
    BOOST_REQUIRE(ind.debug());

    const auto eva_ret(eva(ind));
    BOOST_REQUIRE_LE(eva_ret, vita::fitness_t(1, vita::copies_of, 0.0));

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

  auto eva(vita::make_ga_evaluator<vita::i_ga>(
    [](const std::vector<double> &v)
    { return std::accumulate(v.begin(), v.end(), 0.0); }));

  vita::evolution<vita::i_ga, vita::alps_es> evo1(env, sset, eva);
  BOOST_REQUIRE(evo1.debug(true));

  const auto s1(evo1.run(1));

  BOOST_CHECK_GT(s1.best.solution[0], 8.0);
  BOOST_CHECK_GT(s1.best.solution[1], 95.0);
  BOOST_CHECK_GT(s1.best.solution[2], 990.0);
  BOOST_CHECK_GT(s1.best.solution[3], 9980.0);

  vita::evolution<vita::i_ga, vita::std_es> evo2(env, sset, eva);
  BOOST_REQUIRE(evo2.debug(true));

  const auto s2(evo2.run(1));

  BOOST_CHECK_GT(s2.best.solution[0], 8.0);
  BOOST_CHECK_GT(s2.best.solution[1], 95.0);
  BOOST_CHECK_GT(s2.best.solution[2], 980.0);
  BOOST_CHECK_GT(s2.best.solution[3], 9980.0);
}
BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE(t_ga2, F_FACTORY5_NO_INIT)

// Test problem 1 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem1)
{
  env.individuals = 100;
  env.threshold.fitness = {0, 0};
  env.verbosity = 0;

  vita::problem prob;
  prob.env = env;
  prob.sset.insert(vita::ga::parameter<>(0, 0.0, 6.0));
  prob.sset.insert(vita::ga::parameter<>(1, 0.0, 6.0));

  // The unconstrained objective function f(x1, x2) has a maximum solution at
  // (3, 2) with a function value equal to zero.
  auto f = [](const std::vector<double> &x)
           {
             return -(std::pow(x[0] * x[0] + x[1] - 11, 2.0) +
                      std::pow(x[0] + x[1] * x[1] - 7, 2.0));
           };

  vita::ga_search<vita::i_ga, vita::de_es, decltype(f)> s(prob, f);
  BOOST_REQUIRE(s.debug(true));

  const auto res(s.run().best.solution);

  BOOST_CHECK_SMALL(f(res), 1.0);
  BOOST_CHECK_CLOSE(res[0], 3.0, 1.0);
  BOOST_CHECK_CLOSE(res[1], 2.0, 1.0);

  // Due to the presence of constraints, the previous solution is no more
  // feasible and the constrained optimum solution is (2.246826, 2.381865) with
  // a function value equal to 13.59085. The feasible region is a narrow
  // crescent-shaped region, with the optimum solution lying on the first
  // constraint.
  auto p = [](const vita::i_ga &prg)
    {
      auto g1 = [](const std::vector<double> &x)
      {
        return 4.84 - std::pow(x[0] - 0.05, 2.0) - std::pow(x[1] - 2.5, 2.0);
      };
      auto g2 = [](const std::vector<double> &x)
      {
        return x[0] * x[0] + std::pow(x[1] - 2.5, 2.0) - 4.84;
      };

      int p1(g1(prg) >= 0.0 ? 0 : 1);
      int p2(g2(prg) >= 0.0 ? 0 : 1);
      int p3(0.0 <= prg[0] && prg[0] <= 6.0 ? 0 : 1);
      int p4(0.0 <= prg[1] && prg[1] <= 6.0 ? 0 : 1);

      return p1 + p2 + p3 + p4;
    };

  vita::ga_search<vita::i_ga, vita::de_es, decltype(f)> s2(prob, f, p);
  BOOST_REQUIRE(s2.debug(true));

  const auto res2(s2.run().best.solution);

  BOOST_CHECK_CLOSE(-f(res2), 13.59086, 1.0);
  BOOST_CHECK_CLOSE(res2[0], 2.246826, 1.0);
  BOOST_CHECK_CLOSE(res2[1], 2.381865, 1.0);
}

// Test problem 3 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem3)
{
  env.individuals = 130;
  env.generations = 1000;
  env.threshold.fitness = {0, 0};
  env.verbosity = 0;

  vita::problem prob;
  prob.env = env;

  // Problem's parameters.
  for (unsigned i(0); i < 9; ++i)
    prob.sset.insert(vita::ga::parameter(i, 0.0, 1.0));
  for (unsigned i(9); i < 12; ++i)
    prob.sset.insert(vita::ga::parameter(i, 0.0, 100.0));
  prob.sset.insert(vita::ga::parameter(12, 0.0, 1.0));

  auto f = [](const std::vector<double> &x)
    {
      return
      -(5.0 * (x[0] + x[1] + x[2] + x[3]) -
        5.0 * (x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3]) -
        std::accumulate(std::next(x.begin(), 4), x.end(), 0.0));
    };

  auto p = [](const vita::i_ga &prg)
    {
      auto g1 = [](const std::vector<double> &x)
      {
        return 2.0 * x[0] + 2.0 * x[1] + x[9] + x[10] - 10.0;
      };
      auto g2 = [](const std::vector<double> &x)
      {
        return 2.0 * x[0] * 2 * x[2] + x[9] + x[11] - 10.0;
      };
      auto g3 = [](const std::vector<double> &x)
      {
        return 2.0 * x[1] + 2 * x[2] + x[10] + x[11] - 10.0;
      };
      auto g4 = [](const std::vector<double> &x)
      {
        return -8.0 * x[0] + x[9];
      };
      auto g5 = [](const std::vector<double> &x)
      {
        return -8.0 * x[1] + x[10];
      };
      auto g6 = [](const std::vector<double> &x)
      {
        return -8.0 * x[2] + x[11];
      };
      auto g7 = [](const std::vector<double> &x)
      {
        return -2.0 * x[3] - x[4] + x[9];
      };
      auto g8 = [](const std::vector<double> &x)
      {
        return -2.0 * x[5] - x[6] + x[10];
      };
      auto g9 = [](const std::vector<double> &x)
      {
        return -2.0 * x[7] - x[8] + x[11];
      };

      double r(0.0);

      const auto c1(g1(prg));
      if (c1 > 0.0)  r += c1;

      const auto c2(g2(prg));
      if (c2 > 0.0)  r += c2;

      const auto c3(g3(prg));
      if (c3 > 0.0)  r += c3;

      const auto c4(g4(prg));
      if (c4 > 0.0)  r += c4;

      const auto c5(g5(prg));
      if (c5 > 0.0)  r += c5;

      const auto c6(g6(prg));
      if (c6 > 0.0)  r += c6;

      const auto c7(g7(prg));
      if (c7 > 0.0)  r += c7;

      const auto c8(g8(prg));
      if (c8 > 0.0)  r += c8;

      const auto c9(g9(prg));
      if (c9 > 0.0)  r += c9;

      for (unsigned i(0); i < 9; ++i)
      {
        if (prg[i] < 0.0)
          r += std::abs(prg[i]);
        else if (prg[i] > 1.0)
          r += prg[i] - 1.0;
      }

      for (unsigned i(9); i < 12; ++i)
      {
        if (prg[i] < 0.0)
          r += std::abs(prg[i]);
        else if (prg[i] > 100.0)
          r += prg[i] - 100.0;
      }

      if (prg[12] < 0.0)
        r += std::abs(prg[12]);
      else if (prg[12] > 1.0)
        r += prg[12] - 1.0;

      return r;
    };

  vita::ga_search<vita::i_ga, vita::de_es, decltype(f)> s(prob, f, p);
  BOOST_REQUIRE(s.debug(true));

  const auto res(s.run().best.solution);

  BOOST_REQUIRE_CLOSE(f({1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , 3, 3, 3, 1}), 15, 0.01);

  BOOST_CHECK_CLOSE(-f(res), -15.0, 1.0);
  BOOST_CHECK_CLOSE(res[0], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[1], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[2], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[3], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[4], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[5], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[6], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[7], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[8], 1.0, 1.0);
  BOOST_CHECK_CLOSE(res[9], 3.0, 1.0);
  BOOST_CHECK_CLOSE(res[10], 3.0, 1.0);
  BOOST_CHECK_CLOSE(res[11], 3.0, 1.0);
  BOOST_CHECK_CLOSE(res[12], 1.0, 1.0);
}
BOOST_AUTO_TEST_SUITE_END()
