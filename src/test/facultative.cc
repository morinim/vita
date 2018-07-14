/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2018 EOS di Manlio Morini.
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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("FACULTATIVE")
{

TEST_CASE("Constructor")
{
  using namespace vita;

  const facultative<unsigned> v1;
  CHECK(!v1.has_value());

  const facultative<unsigned> v2(123);
  CHECK(v2.has_value());
  CHECK(*v2 == 123);

  const auto v1_1(v1);
  CHECK(!v1_1.has_value());
  CHECK(*v1 == *v1_1);

  const auto v2_1(v2);
  CHECK(v2_1.has_value());
  CHECK(*v2 == *v2_1);

  const facultative<unsigned> v3(std::numeric_limits<unsigned>::max());
  CHECK(!v3.has_value());

  const facultative<unsigned> v4("1234");
  CHECK(v4.has_value());
  CHECK(*v4 == 1234);

  const facultative<unsigned> v5("dummy");
  CHECK(!v5.has_value());

  const facultative<unsigned> v6 = {};
  CHECK(!v6.has_value());
}

TEST_CASE("Same size")
{
  using namespace vita;

  CHECK(sizeof(facultative<char>) == sizeof(char));
  CHECK(sizeof(facultative<int>) == sizeof(int));
  CHECK(sizeof(facultative<unsigned>) == sizeof(unsigned));
  CHECK(sizeof(facultative<std::int64_t>) == sizeof(std::int64_t));
}

TEST_CASE("Copy assign")
{
  using namespace vita;
  using opt = facultative<int, -1>;

  opt v;
  CHECK(!v.has_value());

  v = opt(123);
  CHECK(v.has_value());
  CHECK(*v == 123);
}

TEST_CASE("Swap facultative")
{
  using namespace vita;
  using opt = facultative<int, -1>;

  const opt::value_type value(456);
  opt original(value), swapped;

  CHECK(original.has_value());
  CHECK(!swapped.has_value());
  CHECK(*original == value);

  swap(original, swapped);

  CHECK(swapped.has_value());
  CHECK(!original.has_value());
  CHECK(*swapped == value);
}

TEST_CASE("Copy operator")
{
  using namespace vita;

  facultative<int> v;
  CHECK(!v.has_value());

  v = 1111;
  CHECK(v.has_value());
  CHECK(*v == 1111);
}

TEST_CASE("null copying")
{
  using namespace vita;

  const facultative<unsigned> null;
  facultative<unsigned> copied = null, assigned;
  assigned = null;

  CHECK(!null.has_value());
  CHECK(!copied.has_value());
  CHECK(!assigned.has_value());
}

TEST_CASE("Reset")
{
  using namespace vita;

  facultative<unsigned> value(456);

  CHECK(value.has_value());

  value.reset();
  CHECK(!value.has_value());

  value.reset();
  CHECK(!value.has_value());  // non-empty after second clear

  value = 456;
  CHECK(value.has_value());

  value.reset();
  CHECK(!value.has_value());
}

TEST_CASE("value_or")
{
  using namespace vita;

  facultative<int, -1> v1, v2(2);

  CHECK(v1.value_or(0) == 0);
  CHECK(v2.value_or(0) == 2);
}

TEST_CASE("nan policy")
{
  using namespace vita;
  using opt = facultative_with_policy<facultative_ns::nan<double>>;

  CHECK(sizeof(opt) == sizeof(double));

  opt v;
  CHECK(!v.has_value());

  v = opt(123.0);
  CHECK(v.has_value());
  CHECK(*v == doctest::Approx(123.0));

  v.reset();
  CHECK(!v.has_value());
}

}  // TEST_SUITE("FACULTATIVE")
