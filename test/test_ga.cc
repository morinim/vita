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
#include "kernel/ga/interpreter.h"
#include "kernel/ga/ga_evaluator.h"
#include "kernel/ga/ga_search.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_ga
#include <boost/test/unit_test.hpp>

constexpr double epsilon(0.00001);

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_ga, F_FACTORY5)

void setup_symbol_set(vita::symbol_set *sset, unsigned n)
{
  double v(10.0);

  for (unsigned i(0); i < n; ++i)
  {
    sset->insert(vita::ga::parameter(i, -v, +v));
    v *= 10.0;
  }
}

BOOST_AUTO_TEST_CASE(Penalty)
{
  setup_symbol_set(&sset, 4);

  vita::i_ga ind(env, sset);
  BOOST_REQUIRE(ind.debug());

  vita::interpreter<vita::i_ga> intr(&ind);

  BOOST_REQUIRE_EQUAL(intr.penalty(), 0);

  ind[0] = 20;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 1);

  ind[1] = 200;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 2);

  ind[2] = 2000;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 3);

  ind[3] = 20000;
  BOOST_REQUIRE_EQUAL(intr.penalty(), 4);
}

BOOST_AUTO_TEST_CASE(Evaluator)
{
  setup_symbol_set(&sset, 4);

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
    BOOST_REQUIRE_LE(eva_ret, vita::fitness_t(1, 0.0));

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
  setup_symbol_set(&sset, 4);

  env.individuals = 100;
  env.verbosity = 0;

  auto eva(vita::make_ga_evaluator<vita::i_ga>(
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

// Test problem 1 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem1)
{
  env.individuals = 100;
  env.f_threashold = {0, 0};
  env.verbosity = 0;

  vita::problem prob;
  prob.env = env;
  prob.sset.insert(vita::ga::parameter(0, 0.0, 6.0));
  prob.sset.insert(vita::ga::parameter(1, 0.0, 6.0));

  vita::ga_search<vita::i_ga, vita::de_es> s(prob);
  BOOST_REQUIRE(s.debug(true));

  // The unconstrained objective function f(x1, x2) has a maximum solution at
  // (3, 2) with a function value equal to zero.
  auto f = [](const std::vector<double> &x)
           {
             return
             -std::pow(x[0]*x[0] + x[1] - 11, 2.0) -
             std::pow(x[0] + x[1] * x[1] - 7, 2.0);
           };

  auto eva(vita::make_unique_ga_evaluator<vita::i_ga>(f));

  s.set_evaluator(std::move(eva));
  const auto res(s.run());

  BOOST_CHECK_SMALL(f(res), epsilon);
  BOOST_CHECK_CLOSE(res[0], 3.0, epsilon);
  BOOST_CHECK_CLOSE(res[1], 2.0, epsilon);

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

  auto c_eva(vita::make_unique_constrained_ga_evaluator<vita::i_ga>(f, p));

  s.set_evaluator(std::move(c_eva));
  const auto res2(s.run());

  BOOST_CHECK_CLOSE(-f(res2), 13.59086, 0.001);
  BOOST_CHECK_CLOSE(res2[0], 2.246826, 0.01);
  BOOST_CHECK_CLOSE(res2[1], 2.381865, 0.01);
}
/*
// Test problem 2 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem2)
{
  env.individuals = 130;
  env.generations = 1000;
  env.f_threashold = {0, 0};
  env.verbosity = 1;

  vita::problem prob;
  prob.env = env;
  for (unsigned i(0); i < 9; ++i)
    prob.sset.insert(vita::ga::parameter(i, 0.0, 1.0));
  for (unsigned i(9); i < 12; ++i)
    prob.sset.insert(vita::ga::parameter(i, 0.0, 100.0));
  prob.sset.insert(vita::ga::parameter(12, 0.0, 1.0));

  vita::ga_search<vita::i_ga, vita::de_es> s(prob);
  BOOST_REQUIRE(s.debug(true));

  // The unconstrained objective function f(x1, x2) has a maximum solution at
  // (3, 2) with a function value equal to zero.
  auto f = [](const std::vector<double> &x)
    {
      return
      -5 * (x[0] + x[1] + x[2] + x[3]) +
      5 * (x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3]) +
      std::accumulate(std::next(x.begin(), 4), x.end(), 0.0);
    };

  auto p = [](const vita::i_ga &prg)
    {
      auto g1 = [](const std::vector<double> &x)
      {
        return 2 * x[0] + 2 * x[1] + x[9] + x[10];
      };
      auto g2 = [](const std::vector<double> &x)
      {
        return 2 * x[0] * 2 * x[2] + x[9] + x[11];
      };
      auto g3 = [](const std::vector<double> &x)
      {
        return 2 * x[1] + 2 * x[2] + x[10] + x[11];
      };
      auto g4 = [](const std::vector<double> &x)
      {
        return -8 * x[0] + x[9];
      };
      auto g5 = [](const std::vector<double> &x)
      {
        return -8 * x[1] + x[10];
      };
      auto g6 = [](const std::vector<double> &x)
      {
        return -8 * x[2] + x[11];
      };
      auto g7 = [](const std::vector<double> &x)
      {
        return -2 * x[3] - x[4] + x[9];
      };
      auto g8 = [](const std::vector<double> &x)
      {
        return -2 * x[5] - x[6] + x[10];
      };
      auto g9 = [](const std::vector<double> &x)
      {
        return -2 * x[7] - x[8] + x[11];
      };

      int r(0);

      r += g1(prg) > 10.0;
      r += g2(prg) > 10.0;
      r += g3(prg) > 10.0;
      r += g4(prg) > 0.0;
      r += g5(prg) > 0.0;
      r += g6(prg) > 0.0;
      r += g7(prg) > 0.0;
      r += g8(prg) > 0.0;
      r += g9(prg) > 0.0;

      for (unsigned i(0); i < 9; ++i)
        r += (prg[i] < 0.0 || prg[i] > 1.0);

      for (unsigned i(9); i < 12; ++i)
        r += (prg[i] < 0.0 || prg[i] > 100.0);

      r += (prg[12] < 0.0 || prg[12] > 1.0);

      return r;
    };

  auto c_eva(vita::make_unique_constrained_ga_evaluator<vita::i_ga>(f, p));

  s.set_evaluator(std::move(c_eva));
  const auto res(s.run());

  BOOST_REQUIRE_CLOSE(f({1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , 3, 3, 3, 1}), 15, 0.01);

  BOOST_CHECK_CLOSE(-f(res), -15.0, 1);
  BOOST_CHECK_CLOSE(res[0], 1.0, 1);
  BOOST_CHECK_CLOSE(res[1], 1.0, 1);
  BOOST_CHECK_CLOSE(res[2], 1.0, 1);
  BOOST_CHECK_CLOSE(res[3], 1.0, 1);
  BOOST_CHECK_CLOSE(res[4], 1.0, 1);
  BOOST_CHECK_CLOSE(res[5], 1.0, 1);
  BOOST_CHECK_CLOSE(res[6], 1.0, 1);
  BOOST_CHECK_CLOSE(res[7], 1.0, 1);
  BOOST_CHECK_CLOSE(res[8], 1.0, 1);
  BOOST_CHECK_CLOSE(res[9], 3.0, 1);
  BOOST_CHECK_CLOSE(res[10], 3.0, 1);
  BOOST_CHECK_CLOSE(res[11], 3.0, 1);
  BOOST_CHECK_CLOSE(res[12], 1.0, 1);
}
*/
/*
// Test problem 7 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem7)
{
  env.individuals = 100;
  env.generations = 7000;
  env.f_threashold = {0, 0};
  env.verbosity = 1;

  vita::problem prob;
  prob.env = env;
  prob.sset.insert(vita::ga::parameter(0, -2.3, 2.3));
  prob.sset.insert(vita::ga::parameter(1, -2.3, 2.3));
  prob.sset.insert(vita::ga::parameter(2, -3.2, 3.2));
  prob.sset.insert(vita::ga::parameter(3, -3.2, 3.2));
  prob.sset.insert(vita::ga::parameter(4, -3.2, 3.2));

  vita::ga_search<vita::i_ga, vita::de_es> s(prob);
  BOOST_REQUIRE(s.debug(true));

  // The unconstrained objective function f(x1, x2) has a maximum solution at
  // (3, 2) with a function value equal to zero.
  auto f = [](const std::vector<double> &x)
           {
             return -std::exp(x[0] * x[1] * x[2] * x[3] * x[4]);
           };

  auto p = [](const vita::i_ga &prg)
    {
      auto h1 = [](const std::vector<double> &x)
      {
        return x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3] + x[4]*x[4];
      };
      auto h2 = [](const std::vector<double> &x)
      {
        return x[1] * x[2] - 5 * x[3] * x[4];
      };
      auto h3 = [](const std::vector<double> &x)
      {
        return x[0]*x[0]*x[0] + x[1]*x[1]*x[1];
      };

      const double delta(0.01);
      int p1(std::abs(h1(prg) - 10.0) <= delta ? 0 : 1);
      int p2(std::abs(h2(prg)) <= delta ? 0 : 1);
      int p3(std::abs(h3(prg) + 1.0) <= delta ? 0 : 1);
      int p4(-2.3 <= prg[0] && prg[0] <= 2.3 ? 0 : 1);
      int p5(-2.3 <= prg[1] && prg[1] <= 2.3 ? 0 : 1);
      int p6(-3.2 <= prg[2] && prg[2] <= 3.2 ? 0 : 1);
      int p7(-3.2 <= prg[3] && prg[3] <= 3.2 ? 0 : 1);
      int p8(-3.2 <= prg[4] && prg[4] <= 3.2 ? 0 : 1);

      return p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
    };

  auto c_eva(vita::make_unique_constrained_ga_evaluator<vita::i_ga>(f, p));

  s.set_evaluator(std::move(c_eva));
  const auto res(s.run(100));

  BOOST_CHECK_CLOSE(-f(res), 0.053950, 0.001);
  BOOST_CHECK_CLOSE(res[0], -1.717143, 0.01);
  BOOST_CHECK_CLOSE(res[1], 1.595709, 0.01);
  BOOST_CHECK_CLOSE(res[2], 1.827247, 0.01);
}
*/
BOOST_AUTO_TEST_SUITE_END()
