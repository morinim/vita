/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <sstream>

#include "kernel/fitness.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE fitness
#include "boost/test/unit_test.hpp"

constexpr double epsilon(0.00001);

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(fitness)

BOOST_AUTO_TEST_CASE(Comparison)
{
  vita::fitness_t fitness2d(2);
  vita::fitness_t fitness3d(3);
  vita::fitness_t fitness4d(4);

  vita::fitness_t f1{3.0, 0.0, 0.0}, f2{2.0, 1.0, 0.0}, f3{2.0, 0.0, 0.0};

  BOOST_CHECK_EQUAL(fitness2d.size(), 2);
  BOOST_CHECK_EQUAL(fitness3d.size(), 3);
  BOOST_CHECK_EQUAL(fitness4d.size(), 4);

  for (unsigned i(0); i < fitness2d.size(); ++i)
    BOOST_CHECK_EQUAL(
      fitness2d[i],
      std::numeric_limits<vita::fitness_t::value_type>::lowest());

  BOOST_CHECK_GT(f1, f2);
  BOOST_CHECK_GE(f1, f2);
  BOOST_CHECK_LT(f2, f1);
  BOOST_CHECK_LE(f2, f1);

  BOOST_CHECK_NE(f1, f2);
  BOOST_CHECK_NE(f2, f1);

  BOOST_CHECK_EQUAL(f1, f1);
  BOOST_CHECK_EQUAL(f2, f2);
  BOOST_CHECK_EQUAL(fitness2d, fitness2d);

  BOOST_CHECK_SMALL(distance(f1, f1), epsilon);
  BOOST_CHECK_SMALL(distance(f2, f2), epsilon);
  BOOST_CHECK_SMALL(distance(fitness2d, fitness2d), epsilon);

  BOOST_CHECK(f1.dominating(fitness3d));
  BOOST_CHECK(!fitness3d.dominating(f1));
  BOOST_CHECK(!f1.dominating(f2));
  BOOST_CHECK(!f2.dominating(f1));
  BOOST_CHECK(!f1.dominating(f1));
  BOOST_CHECK(f1.dominating(f3));
  BOOST_CHECK(f2.dominating(f3));
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  vita::fitness_t f{
    1.0, 2.0, 3.0, std::numeric_limits<vita::fitness_t::value_type>::lowest()};

  std::stringstream ss;

  BOOST_REQUIRE(f.save(ss));

  vita::fitness_t f2(4u);
  BOOST_REQUIRE(f2.load(ss));

  BOOST_CHECK_EQUAL(f, f2);
}

BOOST_AUTO_TEST_CASE(Operators)
{
  vita::fitness_t x{2.0, 4.0, 8.0};
  vita::fitness_t f1{2.0, 4.0, 8.0};
  vita::fitness_t f2{4.0, 8.0, 16.0};
  vita::fitness_t inf(
    3, std::numeric_limits<vita::fitness_t::value_type>::infinity());

  x += x;
  BOOST_CHECK_EQUAL(x, f2);

  BOOST_CHECK_EQUAL(x / 2.0, f1);

  BOOST_CHECK_EQUAL(f1 * 2.0, f2);

  x = f1 * vita::fitness_t{2.0, 2.0, 2.0};
  BOOST_CHECK_EQUAL(x, f2);

  x += vita::fitness_t{0.0, 0.0, 0.0};
  BOOST_CHECK_EQUAL(x, f2);

  x = x / 1.0;
  BOOST_CHECK_EQUAL(x, f2);

  x = f2 - f1;
  BOOST_CHECK_EQUAL(x, f1);

  x = x * x;
  x = sqrt(x);
  BOOST_CHECK_EQUAL(x, f1);

  x = x * -1.0;
  x = abs(x);
  BOOST_CHECK_EQUAL(f1, x);

  BOOST_CHECK(isfinite(x));
  BOOST_CHECK(!isfinite(inf));
}

BOOST_AUTO_TEST_CASE(Joining)
{
  const vita::fitness_t f1{1.0, 2.0, 3.0}, f2{4.0, 5.0, 6.0};

  const vita::fitness_t f3(combine(f1, f2));
  const vita::fitness_t f4{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  BOOST_CHECK_EQUAL(f3, f4);
}

BOOST_AUTO_TEST_SUITE_END()
