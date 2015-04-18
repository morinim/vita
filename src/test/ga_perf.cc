/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
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
#define BOOST_TEST_MODULE t_ga_perf
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_ga2, F_FACTORY5_NO_INIT)

// Test problem 7 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem7)
{
  env.individuals = 100;
  env.generations = 2000;
  env.f_threashold = {0, 0};
  env.verbosity = 1;

  vita::problem prob;
  prob.env = env;
  prob.env.stat_dir = ".";
  prob.env.stat_layers = true;
  prob.sset.insert(vita::ga::parameter(0, -2.3, 2.3));
  prob.sset.insert(vita::ga::parameter(1, -2.3, 2.3));
  prob.sset.insert(vita::ga::parameter(2, -3.2, 3.2));
  prob.sset.insert(vita::ga::parameter(3, -3.2, 3.2));
  prob.sset.insert(vita::ga::parameter(4, -3.2, 3.2));

  auto f = [](const std::vector<double> &x)
           {
             return -std::exp(x[0] * x[1] * x[2] * x[3] * x[4]);
           };

  auto p = [](const vita::i_ga &prg)
    {
      auto h1 = [](const std::vector<double> &x)
      {
        return x[0] * x[0] + x[1] * x[1] + x[2] * x[2] + x[3] * x[3] +
               x[4] * x[4] - 10.0;
      };
      auto h2 = [](const std::vector<double> &x)
      {
        return x[1] * x[2] - 5.0 * x[3] * x[4];
      };
      auto h3 = [](const std::vector<double> &x)
      {
        return x[0] * x[0] * x[0] + x[1] * x[1] * x[1] + 1.0;
      };

      const double delta(0.01);

      double r(0.0);

      const auto c1(std::abs(h1(prg)));
      if (c1 > delta)
        r += c1;

      const auto c2(std::abs(h2(prg)));
      if (c2 > delta)
        r += c2;

      const auto c3(std::abs(h3(prg)));
      if (c3 > delta)
        r += c3;

      for (unsigned i(0), size(prg.size()); i < size; ++i)
      {
        if (prg[i] < -2.3)
          r += -2.3 - prg[i];
        else if (prg[i] > 3.2)
          r += prg[i] - 3.2;
      }

      return r;
    };

  vita::ga_search<vita::i_ga, vita::de_es, decltype(f)> s(prob, f, p);
  BOOST_REQUIRE(s.debug(true));
  const auto res(s.run(10));

  BOOST_CHECK_CLOSE(-f(res), 0.053950, 2.0);
  BOOST_CHECK_CLOSE(res[0], -1.717143, 1.0);
  BOOST_CHECK_CLOSE(res[1], 1.595709, 1.0);
  BOOST_CHECK_CLOSE(res[2], 1.827247, 1.0);
}

BOOST_AUTO_TEST_SUITE_END()
