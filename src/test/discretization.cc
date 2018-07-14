/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016, 2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/random.h"
#include "utility/discretization.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("DISCRETIZATION")
{

TEST_CASE("sigmoid_01")
{
  using vita::sigmoid_01;

  CHECK(sigmoid_01(0.0) == doctest::Approx(0.5));
  CHECK(sigmoid_01(std::numeric_limits<double>::infinity())
        == doctest::Approx(1.0));
  CHECK(sigmoid_01(std::numeric_limits<double>::max())
        == doctest::Approx(1.0));
  CHECK(sigmoid_01(-std::numeric_limits<double>::infinity())
        == doctest::Approx(0.0));
  CHECK(sigmoid_01(std::numeric_limits<double>::lowest())
        == doctest::Approx(0.0));
}

TEST_CASE("Max discretization")
{
  using vita::discretization;

  const auto ri(vita::random::between(-10000.0, 10000.0));
  CHECK(discretization(ri, 10u)
        == doctest::Approx(discretization(ri, 0u, 10u)));
}

TEST_CASE("Discretization")
{
  using vita::discretization;

  CHECK(discretization(std::numeric_limits<double>::infinity(), -1, 1) == 1);
  CHECK(discretization(-std::numeric_limits<double>::infinity(), -1, 1) == -1);
  CHECK(discretization(0.0, -1, 1) == 0);

  for (double x(0.0); x < 1000.0; ++x)
  {
    const auto s(discretization(x, 1u, 10u));

    CHECK(1 <= s);
    CHECK(s <= 10);
    CHECK(s >= discretization(x - 1.0, 0u, 10u));
  }
}

TEST_CASE("Discretization bound")
{
  using vita::discretization;

  for (double x(-1000.0); x < 1000.0; ++x)
  {
    const auto s(discretization(x, -1000.0, 1000.0, -1000, 1000));

    CHECK(static_cast<int>(x) == s);
  }

  CHECK(discretization(1000.0, 0.0, 500.0, 0u, 10u) == 10);
  CHECK(discretization( -10.0, 0.0, 500.0, 0u, 10u) ==  0);
}

}  // TEST_SUITE("DISCRETIZATION")
