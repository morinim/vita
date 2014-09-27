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

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(fitness)

BOOST_AUTO_TEST_CASE(Comparison)
{
  using fitness2_t = vita::basic_fitness_t<double, 2>;
  using fitness3_t = vita::basic_fitness_t<double, 3>;
  using fitness4_t = vita::basic_fitness_t<double, 4>;

  fitness2_t fitness2d;
  fitness3_t fitness3d;
  fitness4_t fitness4d;

  fitness3_t f1(3.0, 0.0, 0.0), f2(2.0, 1.0, 0.0), f3(2.0, 0.0, 0.0);

  BOOST_CHECK_EQUAL(fitness2_t::size, 2);
  BOOST_CHECK_EQUAL(fitness3_t::size, 3);
  BOOST_CHECK_EQUAL(fitness4_t::size, 4);

  for (unsigned i(0); i < fitness2_t::size; ++i)
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
  using fitness4_t = vita::basic_fitness_t<double, 4>;

  fitness4_t f(1.0, 2.0, 3.0,
               std::numeric_limits<vita::fitness_t::value_type>::lowest());

  std::stringstream ss;

  BOOST_REQUIRE(f.save(ss));

  fitness4_t f2;
  BOOST_REQUIRE(f2.load(ss));

  BOOST_CHECK_EQUAL(f, f2);
}

BOOST_AUTO_TEST_CASE(Operators)
{
  using fitness3_t = vita::basic_fitness_t<double, 3>;

  fitness3_t x(2.0, 4.0, 8.0);
  fitness3_t f1(2.0, 4.0, 8.0);
  fitness3_t f2(4.0, 8.0, 16.0);
  fitness3_t inf(std::numeric_limits<vita::fitness_t::value_type>::infinity());

  x += x;
  BOOST_CHECK_EQUAL(x, f2);

  BOOST_CHECK_EQUAL(x / 2.0, f1);

  BOOST_CHECK_EQUAL(f1 * 2.0, f2);

  x = f1 * fitness3_t(2.0, 2.0, 2.0);
  BOOST_CHECK_EQUAL(x, f2);

  x += fitness3_t(0.0, 0.0, 0.0);
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

BOOST_AUTO_TEST_SUITE_END()
