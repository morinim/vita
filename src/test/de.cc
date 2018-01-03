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
#include "kernel/ga/i_de.h"
#include "kernel/ga/search.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_de
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_de1, F_FACTORY5)

BOOST_AUTO_TEST_CASE(Evaluator)
{
  using namespace vita;

  auto f = [](const std::vector<double> &v)
           { return std::accumulate(v.begin(), v.end(), 0.0); };

  auto eva(make_ga_evaluator<i_de>(f));

  fitness_t eva_prev;
  double v_prev(0.0);

  for (unsigned i(0); i < 1000; ++i)
  {
    i_de ind(prob);
    BOOST_TEST(ind.debug());

    const auto eva_ret(eva(ind));

    const auto v(f(ind));

    if (i)
    {
      if (v_prev < v)
        BOOST_TEST(eva_prev < eva_ret);
      else
        BOOST_TEST(eva_prev >= eva_ret);
    }

    v_prev = v;
    eva_prev = eva_ret;
  }
}
BOOST_AUTO_TEST_SUITE_END()



BOOST_FIXTURE_TEST_SUITE(t_de2, F_FACTORY5_NO_INIT)
// Test problem 1 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem1, * boost::unit_test::tolerance(1.0))
{
  using namespace vita;
  print.verbosity(vita::log::L_WARNING);

  prob.env.individuals = 120;
  prob.env.threshold.fitness = {0,0};
  prob.sset.insert<ga::real>(range(0.0, 6.0));
  prob.sset.insert<ga::real>(range(0.0, 6.0));

  // The unconstrained objective function f(x1, x2) has a maximum solution at
  // (3, 2) with a function value equal to zero.
  auto f = [](const std::vector<double> &x)
           {
             return -(std::pow(x[0] * x[0] + x[1] - 11, 2.0) +
                      std::pow(x[0] + x[1] * x[1] - 7, 2.0));
           };
  de_search<decltype(f)> s(prob, f);
  BOOST_TEST(s.debug());

  const auto res(s.run().best.solution);

  BOOST_TEST(f(res) == 0.0);
  BOOST_TEST(res[0] == 3.0);
  BOOST_TEST(res[1] == 2.0);

  // Due to the presence of constraints, the previous solution is no more
  // feasible and the constrained optimum solution is (2.246826, 2.381865) with
  // a function value equal to 13.59085. The feasible region is a narrow
  // crescent-shaped region, with the optimum solution lying on the first
  // constraint.
  auto p = [](const i_de &prg)
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

  de_search<decltype(f)> s2(prob, f, p);
  BOOST_TEST(s2.debug());

  const auto res2(s2.run().best.solution);

  BOOST_TEST(-f(res2) == 13.59086);
  BOOST_TEST(res2[0] == 2.246826);
  BOOST_TEST(res2[1] == 2.381865);
}

// Test problem 3 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem3, * boost::unit_test::tolerance(1.0))
{
  using namespace vita;

  print.verbosity(log::L_WARNING);

  prob.env.individuals = 130;
  prob.env.generations = 1000;
  prob.env.threshold.fitness = {0, 0};

  // Problem's parameters.
  for (unsigned i(0); i < 9; ++i)
    prob.sset.insert<ga::real>(range(0.0, 1.0));
  for (unsigned i(9); i < 12; ++i)
    prob.sset.insert<ga::real>(range(0.0, 100.0));
  prob.sset.insert<ga::real>(range(0.0, 1.0));

  auto f = [](const std::vector<double> &x)
    {
      return
      -(5.0 * (x[0] + x[1] + x[2] + x[3]) -
        5.0 * (x[0]*x[0] + x[1]*x[1] + x[2]*x[2] + x[3]*x[3]) -
        std::accumulate(std::next(x.begin(), 4), x.end(), 0.0));
    };

  auto p = [](const i_de &prg)
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

  de_search<decltype(f)> s(prob, f, p);
  BOOST_TEST(s.debug());

  const auto res(s.run().best.solution);

  BOOST_TEST(f({1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 , 3, 3, 3, 1}) == 15.0,
             boost::test_tools::tolerance(0.01));

  BOOST_TEST(-f(res) == -15.0);
  BOOST_TEST(res[ 0] == 1.0);
  BOOST_TEST(res[ 1] == 1.0);
  BOOST_TEST(res[ 2] == 1.0);
  BOOST_TEST(res[ 3] == 1.0);
  BOOST_TEST(res[ 4] == 1.0);
  BOOST_TEST(res[ 5] == 1.0);
  BOOST_TEST(res[ 6] == 1.0);
  BOOST_TEST(res[ 7] == 1.0);
  BOOST_TEST(res[ 8] == 1.0);
  BOOST_TEST(res[ 9] == 3.0);
  BOOST_TEST(res[10] == 3.0);
  BOOST_TEST(res[11] == 3.0);
  BOOST_TEST(res[12] == 1.0);
}

// Test problem from <http://stackoverflow.com/q/36230735/3235496>
BOOST_AUTO_TEST_CASE(Search_TestProblem4)
{
  using namespace vita;
  print.verbosity(log::L_WARNING);

  prob.env.individuals = 50;
  prob.env.generations = 1000;
  prob.env.threshold.fitness = {0, 0};

  // Problem's parameters.
  prob.sset.insert<ga::real>(range(0.0, 100.0));
  prob.sset.insert<ga::real>(range(0.0, 100.0));

  auto f = [](const std::vector<double> &x)
    {
      return
      x[0] - (std::max(0.0, x[0] - 50.0) * x[1]) +
      std::max(0.0, x[0] - 75.0) * 2 * x[1];
    };

  auto p = [](const i_de &prg)
    {
      double r(0.0);

      for (unsigned i(0); i < 2; ++i)
      {
        if (prg[i] < 0.0)
          r += -prg[i];
        else if(prg[i] > 100.0)
          r += prg[i] - 100.0;
      }

      return r;
    };

  de_search<decltype(f)> s(prob, f, p);
  BOOST_TEST(s.debug());

  const auto res(s.run().best.solution);

  BOOST_TEST(f(res) == 100.0, boost::test_tools::tolerance(0.1));
}

// Test problem based on Langermann function (multimodal, with many unevely
// distributed local minima).
BOOST_AUTO_TEST_CASE(Search_TestProblem5, * boost::unit_test::tolerance(0.1))
{
  using namespace vita;
  print.verbosity(log::L_WARNING);

  prob.env.individuals = 100;
  prob.env.generations = 500;
  prob.env.threshold.fitness = {0, 0};

  // Problem's parameters.
  for (unsigned i(0); i < 2; ++i)
    prob.sset.insert<ga::real>(range(0.0, 10.0));

  auto f = [](const std::vector<double> &x)
    {
      const double A[5][2] = {{3.0, 5.0}, {5.0, 2.0}, {2.0, 1.0},
                              {1.0, 4.0}, {7.0, 9.0}};
      const double c[5] = {1.0, 2.0, 5.0, 2.0, 3.0};
      const double pi(3.1415926535897932);
      const unsigned d(2);
      const unsigned m(5);

      double ret(0.0);

      for (unsigned i(0); i < m; ++i)
      {
        double s(0.0);
        for (unsigned j(0); j < d; ++j)
          s += std::pow(x[j] - A[i][j], 2.0);

        ret += c[i] * std::exp(-s / pi) * std::cos(pi * s);
      }

      return ret;
    };
  assert(std::abs(f({2.00299219, 1.006096}) - 5.1621259) < 0.001);

  auto p = [](const i_de &prg)
    {
      double r(0.0);

      for (unsigned i(0); i < 2; ++i)
      {
        if (prg[i] < 0.0)
          r += -prg[i];
        else if(prg[i] > 10.0)
          r += prg[i] - 10.0;
      }

      return r;
    };

  de_search<decltype(f)> s(prob, f, p);
  BOOST_TEST(s.debug());

  const auto res(s.run().best.solution);

  BOOST_TEST(f(res) == 5.1621259);
  BOOST_TEST(res[0] == 2.00299219);
  BOOST_TEST(res[1] == 1.006096);
}

BOOST_AUTO_TEST_SUITE_END()
