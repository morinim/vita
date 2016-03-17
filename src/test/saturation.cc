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
#include "utility/saturation.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE SATURATION
#include <boost/test/unit_test.hpp>

using namespace boost;

#endif

BOOST_AUTO_TEST_SUITE(saturation)

BOOST_AUTO_TEST_CASE(is_in_range)
{
  using std::numeric_limits;
  using vita::is_in_range;

  BOOST_TEST(is_in_range<int>(numeric_limits<int>::max()));
  BOOST_TEST(is_in_range<int>(numeric_limits<int>::min()));

  BOOST_TEST(!is_in_range<int>(numeric_limits<long long>::max()));
  BOOST_TEST(!is_in_range<int>(numeric_limits<long long>::min()));

  BOOST_TEST(!is_in_range<int>(numeric_limits<unsigned>::max()));
  BOOST_TEST(is_in_range<int>(0u));

  BOOST_TEST(is_in_range<int>(static_cast<double>(3.14)));
  BOOST_TEST(is_in_range<int>(static_cast<double>(-6.28)));
  BOOST_TEST(is_in_range<int>(numeric_limits<double>::min()));
  BOOST_TEST(!is_in_range<int>(numeric_limits<double>::max()));
  BOOST_TEST(!is_in_range<int>(numeric_limits<double>::lowest()));

  BOOST_TEST(is_in_range<double>(numeric_limits<int>::max()));
  BOOST_TEST(is_in_range<double>(numeric_limits<unsigned>::max()));
  BOOST_TEST(is_in_range<double>(numeric_limits<unsigned long>::max()));
  BOOST_TEST(is_in_range<double>(numeric_limits<long long>::max()));
  BOOST_TEST(is_in_range<double>(numeric_limits<unsigned long long>::max()));
  BOOST_TEST(is_in_range<double>(numeric_limits<int>::min()));
  BOOST_TEST(is_in_range<double>(numeric_limits<long long>::min()));
  BOOST_TEST(is_in_range<double>(numeric_limits<float>::max()));
  BOOST_TEST(is_in_range<double>(numeric_limits<float>::lowest()));
  BOOST_TEST(!is_in_range<double>(numeric_limits<long double>::max()));
  BOOST_TEST(!is_in_range<double>(numeric_limits<long double>::lowest()));
  BOOST_TEST(is_in_range<double>(static_cast<long double>(1E300)));
  BOOST_TEST(is_in_range<double>(static_cast<long double>(-1E300)));
  BOOST_TEST(!is_in_range<float>(
               static_cast<double>(numeric_limits<float>::max()) * 10.0));
}

BOOST_AUTO_TEST_CASE(sigmoid_01, * unit_test::tolerance(0.00001))
{
  using vita::sigmoid_01;

  BOOST_TEST(sigmoid_01(0.0) == 0.5);
  BOOST_TEST(sigmoid_01(std::numeric_limits<double>::infinity()) == 1.0);
  BOOST_TEST(sigmoid_01(std::numeric_limits<double>::max()) == 1.0);
  BOOST_TEST(sigmoid_01(-std::numeric_limits<double>::infinity()) == 0.0);
  BOOST_TEST(sigmoid_01(std::numeric_limits<double>::lowest()) == 0.0);
}

BOOST_AUTO_TEST_CASE(max_saturation)
{
  namespace tt = boost::test_tools;
  using vita::max_saturation;
  using vita::saturation;

  const auto ri(vita::random::between(-10000.0, 10000.0));
  BOOST_TEST(max_saturation(ri, 10u) == saturation(ri, 0u, 10u),
             tt::tolerance(0.00001));
}

BOOST_AUTO_TEST_CASE(saturation, * unit_test::tolerance(0.00001))
{
  using vita::saturation;

  BOOST_TEST(saturation(std::numeric_limits<double>::infinity(), -1.0, 1.0)
             == 1.0);
  BOOST_TEST(saturation(-std::numeric_limits<double>::infinity(), -1.0, 1.0)
             == -1.0);
  BOOST_TEST(saturation(0, -1.0, 1.0) == 0.0);

  for (double x(0.0); x < 1000.0; ++x)
  {
    const auto s(saturation(x, 1u, 10u));

    BOOST_TEST((1 <= s && s <= 10));
    BOOST_TEST(s >= saturation(x - 1.0, 0u, 10u));
  }
}

BOOST_AUTO_TEST_SUITE_END()
