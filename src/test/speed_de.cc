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
#include <sstream>

#include "kernel/ga/evaluator.h"
#include "kernel/ga/i_de.h"
#include "kernel/ga/search.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

#include "utility/timer.h"

#include "test/fixture5.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("DE SPEED TEST")
{

// Test problem 7 from "An Efficient Constraint Handling Method for Genetic
// Algorithms"
TEST_CASE_FIXTURE(fixture5_no_init, "Test Problem 7")
{
  using namespace vita;

  log::reporting_level = log::lWARNING;

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
  CHECK(s.is_valid());

  vita::timer t;
  const auto res(s.run(10).best.solution);
  std::cout << "Elapsed: " << t.elapsed().count() << "ms\n";

  CHECK(-f(res) == doctest::Approx(0.053950).epsilon(0.01));
  CHECK(res[0] == doctest::Approx(-1.717143).epsilon(0.01));
  CHECK(res[1] == doctest::Approx(1.595709).epsilon(0.01));
  CHECK(res[2] == doctest::Approx(1.827247).epsilon(0.01));
}

}  // TEST_SUITE("DE SPEED TEST")
