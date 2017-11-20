/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>
#include <vector>

#include "kernel/random.h"
#include "utility/facultative.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE Facultative
#include <boost/test/unit_test.hpp>

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(test_facultative)

BOOST_AUTO_TEST_CASE(ctor)
{
  using namespace vita;

  const facultative<unsigned> v1;
  BOOST_TEST(!v1.has_value());

  const facultative<unsigned> v2(123);
  BOOST_TEST(v2.has_value());
  BOOST_TEST(*v2 == 123);

  const auto v1_1(v1);
  BOOST_TEST(!v1_1.has_value());
  BOOST_TEST(*v1 == *v1_1);

  const auto v2_1(v2);
  BOOST_TEST(v2_1.has_value());
  BOOST_TEST(*v2 == *v2_1);

  const facultative<unsigned> v3(std::numeric_limits<unsigned>::max());
  BOOST_TEST(!v3.has_value());

  const facultative<unsigned> v4("1234");
  BOOST_TEST(v4.has_value());
  BOOST_TEST(*v4 == 1234);

  const facultative<unsigned> v5("dummy");
  BOOST_TEST(!v5.has_value());

  const facultative<unsigned> v6 = {};
  BOOST_TEST(!v6.has_value());
}

BOOST_AUTO_TEST_CASE(same_size)
{
  using namespace vita;

  BOOST_TEST(sizeof(facultative<char>) == sizeof(char));
  BOOST_TEST(sizeof(facultative<int>) == sizeof(int));
  BOOST_TEST(sizeof(facultative<unsigned>) == sizeof(unsigned));
  BOOST_TEST(sizeof(facultative<std::int64_t>) == sizeof(std::int64_t));
}

BOOST_AUTO_TEST_CASE(copy_assign)
{
  using namespace vita;
  using opt = facultative<int, -1>;

  opt v;
  BOOST_TEST(!v.has_value());

  v = opt(123);
  BOOST_TEST(v.has_value());
  BOOST_TEST(*v == 123);
}

BOOST_AUTO_TEST_CASE(swap_facultative)
{
  using namespace vita;
  using opt = facultative<int, -1>;

  const opt::value_type value(456);
  opt original(value), swapped;

  BOOST_TEST(original.has_value());
  BOOST_TEST(!swapped.has_value());
  BOOST_TEST(*original == value);

  swap(original, swapped);

  BOOST_TEST(swapped.has_value());
  BOOST_TEST(!original.has_value());
  BOOST_TEST(*swapped == value);
}

BOOST_AUTO_TEST_CASE(copy_operator)
{
  using namespace vita;

  facultative<int> v;
  BOOST_TEST(!v.has_value());

  v = 1111;
  BOOST_TEST(v.has_value());
  BOOST_TEST(*v == 1111);
}

BOOST_AUTO_TEST_CASE(null_copying)
{
  using namespace vita;

  const facultative<unsigned> null;
  facultative<unsigned> copied = null, assigned;
  assigned = null;

  BOOST_TEST(!null.has_value());
  BOOST_TEST(!copied.has_value());
  BOOST_TEST(!assigned.has_value());
}

BOOST_AUTO_TEST_CASE(reset)
{
  using namespace vita;

  facultative<unsigned> value(456);

  BOOST_TEST(value.has_value());

  value.reset();
  BOOST_TEST(!value.has_value());

  value.reset();
  BOOST_TEST(!value.has_value());  // non-empty after second clear

  value = 456;
  BOOST_TEST(value.has_value());

  value.reset();
  BOOST_TEST(!value.has_value());
}

BOOST_AUTO_TEST_CASE(value_or)
{
  facultative<int, -1> v1, v2(2);

  BOOST_TEST(v1.value_or(0) == 0);
  BOOST_TEST(v2.value_or(0) == 2);
}

BOOST_AUTO_TEST_CASE(facultative_nan_policy)
{
  using namespace vita;
  using opt = facultative_with_policy<facultative_ns::nan<double>>;

  BOOST_TEST(sizeof(opt) == sizeof(double));

  opt v;
  BOOST_TEST(!v.has_value());

  v = opt(123.0);
  BOOST_TEST(v.has_value());
  BOOST_TEST(*v == 123.0);

  v.reset();
  BOOST_TEST(!v.has_value());
}

BOOST_AUTO_TEST_SUITE_END()
