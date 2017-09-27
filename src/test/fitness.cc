/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
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
  using namespace vita;

  fitness_t fitness2d(2, copies_of);
  fitness_t fitness3d(3, copies_of);
  fitness_t fitness4d(4, copies_of);

  fitness_t f1{3.0, 0.0, 0.0}, f2{2.0, 1.0, 0.0}, f3{2.0, 0.0, 0.0};

  BOOST_TEST(fitness2d.size() == 2);
  BOOST_TEST(fitness3d.size() == 3);
  BOOST_TEST(fitness4d.size() == 4);

  for (const auto &v : fitness2d)
    BOOST_TEST(v == std::numeric_limits<fitness_t::value_type>::lowest());

  BOOST_TEST(f1 > f2);
  BOOST_TEST(f1 >= f2);
  BOOST_TEST(f2 < f1);
  BOOST_TEST(f2 <= f1);

  BOOST_TEST(f1 != f2);
  BOOST_TEST(f2 != f1);

  BOOST_TEST(f1 == f1);
  BOOST_TEST(f2 == f2);
  BOOST_TEST(fitness2d == fitness2d);

  BOOST_CHECK_SMALL(distance(f1, f1), epsilon);
  BOOST_CHECK_SMALL(distance(f2, f2), epsilon);
  BOOST_CHECK_SMALL(distance(fitness2d, fitness2d), epsilon);

  BOOST_TEST(dominating(f1, fitness3d));
  BOOST_TEST(!dominating(fitness3d, f1));
  BOOST_TEST(!dominating(f1, f2));
  BOOST_TEST(!dominating(f2, f1));
  BOOST_TEST(!dominating(f1, f1));
  BOOST_TEST(dominating(f1, f3));
  BOOST_TEST(dominating(f2, f3));
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  using namespace vita;

  const fitness_t f{1.0, 2.0, 3.0,
                    std::numeric_limits<fitness_t::value_type>::lowest()};

  std::stringstream ss;

  BOOST_TEST(f.save(ss));

  fitness_t f2;
  BOOST_TEST(f2.size() == 0);
  BOOST_TEST(f2.load(ss));

  BOOST_TEST(f2.size() == 4);
  BOOST_TEST(f == f2);
}

BOOST_AUTO_TEST_CASE(Operators, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;

  fitness_t x{2.0, 4.0, 8.0};
  fitness_t f1{2.0, 4.0, 8.0};
  fitness_t f2{4.0, 8.0, 16.0};
  fitness_t inf(3, copies_of,
                std::numeric_limits<fitness_t::value_type>::infinity());

  x += x;
  BOOST_TEST(x == f2);

  BOOST_TEST(x / 2.0 == f1);

  BOOST_TEST(f1 * 2.0 == f2);

  x = f1 * fitness_t{2.0, 2.0, 2.0};
  BOOST_TEST(x == f2);

  x += {0.0, 0.0, 0.0};
  BOOST_TEST(x == f2);

  x = x / 1.0;
  BOOST_TEST(x == f2);

  x = f2 - f1;
  BOOST_TEST(x == f1);

  x = x * x;
  x = sqrt(x);
  BOOST_TEST(x == f1);

  x = x * -1.0;
  x = abs(x);
  BOOST_TEST(f1 == x);

  BOOST_TEST(isfinite(x));
  BOOST_TEST(!isfinite(inf));
}

BOOST_AUTO_TEST_CASE(Joining)
{
  using namespace vita;

  const fitness_t f1{1.0, 2.0, 3.0}, f2{4.0, 5.0, 6.0};

  const fitness_t f3(combine(f1, f2));
  const fitness_t f4{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  BOOST_TEST(f3 == f4);
}

BOOST_AUTO_TEST_SUITE_END()
