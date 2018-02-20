/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2018 EOS di Manlio Morini.
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
#define BOOST_TEST_MODULE t_de_perf
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture5.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_de7, F_FACTORY5_NO_INIT)

// Test problem 7 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
BOOST_AUTO_TEST_CASE(Search_TestProblem7, * boost::unit_test::tolerance(1.0))
{
  using namespace vita;

  log::reporting_level = log::WARNING;

  prob.env.individuals = 100;
  prob.env.generations = 2000;
  prob.env.threshold.fitness = {0, 0};
  prob.sset.insert<ga::real>(vita::range(-2.3, 2.3));
  prob.sset.insert<ga::real>(vita::range(-2.3, 2.3));
  prob.sset.insert<ga::real>(vita::range(-3.2, 3.2));
  prob.sset.insert<ga::real>(vita::range(-3.2, 3.2));
  prob.sset.insert<ga::real>(vita::range(-3.2, 3.2));

  auto f = [](const std::vector<double> &x)
  {
    return -std::exp(x[0] * x[1] * x[2] * x[3] * x[4]);
  };

  auto p = [](const i_de &prg)
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

    for (const auto &pi : prg)
    {
      if (pi < -2.3)
        r += -2.3 - pi;
      else if (pi > 3.2)
        r += pi - 3.2;
    }

    return r;
  };

  de_search<decltype(f)> s(prob, f, p);
  BOOST_TEST(s.debug());
  const auto res(s.run(10).best.solution);

  BOOST_TEST(-f(res) == 0.053950);
  BOOST_TEST(res[0] == -1.717143);
  BOOST_TEST(res[1] == 1.595709);
  BOOST_TEST(res[2] == 1.827247);
}

BOOST_AUTO_TEST_SUITE_END()
