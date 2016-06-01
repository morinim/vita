/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
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

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE DISCRETIZATION
#include <boost/test/unit_test.hpp>

using namespace boost;

#endif

BOOST_AUTO_TEST_SUITE(discretization)

BOOST_AUTO_TEST_CASE(sigmoid_01, * unit_test::tolerance(0.00001))
{
  using vita::sigmoid_01;

  BOOST_TEST(sigmoid_01(0.0) == 0.5);
  BOOST_TEST(sigmoid_01(std::numeric_limits<double>::infinity()) == 1.0);
  BOOST_TEST(sigmoid_01(std::numeric_limits<double>::max()) == 1.0);
  BOOST_TEST(sigmoid_01(-std::numeric_limits<double>::infinity()) == 0.0);
  BOOST_TEST(sigmoid_01(std::numeric_limits<double>::lowest()) == 0.0);
}

BOOST_AUTO_TEST_CASE(max_discretization)
{
  namespace tt = boost::test_tools;
  using vita::discretization;

  const auto ri(vita::random::between(-10000.0, 10000.0));
  BOOST_TEST(discretization(ri, 10u) == discretization(ri, 0u, 10u),
             tt::tolerance(0.00001));
}

BOOST_AUTO_TEST_CASE(discretization)
{
  using vita::discretization;

  BOOST_TEST(discretization(std::numeric_limits<double>::infinity(), -1, 1)
             == 1);
  BOOST_TEST(discretization(-std::numeric_limits<double>::infinity(), -1, 1)
             == -1);
  BOOST_TEST(discretization(0.0, -1, 1) == 0);

  for (double x(0.0); x < 1000.0; ++x)
  {
    const auto s(discretization(x, 1u, 10u));

    BOOST_TEST((1 <= s && s <= 10));
    BOOST_TEST(s >= discretization(x - 1.0, 0u, 10u));
  }
}

BOOST_AUTO_TEST_CASE(discretization_bound, * unit_test::tolerance(0.00001))
{
  using vita::discretization;

  for (double x(-1000.0); x < 1000.0; ++x)
  {
    const auto s(discretization(x, -1000.0, 1000.0, -1000, 1000));

    BOOST_TEST(static_cast<int>(x) == s);
  }

  BOOST_TEST(discretization(1000.0, 0.0, 500.0, 0u, 10u) == 10);
  BOOST_TEST(discretization( -10.0, 0.0, 500.0, 0u, 10u) ==  0);
}

BOOST_AUTO_TEST_SUITE_END()
