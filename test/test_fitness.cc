/**
 *
 *  \file test_fitness.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "fitness.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE fitness_t
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(fitness_t)

BOOST_AUTO_TEST_CASE(Comparison)
{
  vita::fitness_t empty;
  vita::fitness_t fitness2d(2), fitness3d(3), fitness4d(4);
  vita::fitness_t f1{3.0, 0.0, 0.0}, f2{2.0, 1.0, 0.0};

  BOOST_CHECK(empty.empty());
  BOOST_CHECK_EQUAL(empty.size(), 0);
  BOOST_CHECK(!fitness2d.empty());
  BOOST_CHECK_EQUAL(fitness2d.size(), 2);
  BOOST_CHECK_EQUAL(fitness3d.size(), 3);
  BOOST_CHECK_EQUAL(fitness4d.size(), 4);

  BOOST_CHECK_EQUAL(fitness2d.size(), 2);
  for (size_t i(0); i < fitness2d.size(); ++i)
    BOOST_CHECK_EQUAL(fitness2d[i],
                      std::numeric_limits<vita::fitness_t::base_t>::lowest());

  BOOST_CHECK_GT(f1, f2);
  BOOST_CHECK_GE(f1, f2);
  BOOST_CHECK_LT(f2, f1);
  BOOST_CHECK_LE(f2, f1);

  BOOST_CHECK_NE(f1, f2);
  BOOST_CHECK_NE(f2, f1);

  BOOST_CHECK_EQUAL(f1, f1);
  BOOST_CHECK_EQUAL(f2, f2);
  BOOST_CHECK_EQUAL(empty, empty);
  BOOST_CHECK_EQUAL(fitness2d, fitness2d);

  BOOST_CHECK(f1.dominating(empty));
  BOOST_CHECK(f1.dominating(fitness2d));
  BOOST_CHECK(f1.dominating(fitness3d));
  BOOST_CHECK(!f1.dominating(fitness4d));
  BOOST_CHECK(!empty.dominating(f1));
  BOOST_CHECK(!fitness2d.dominating(f1));
  BOOST_CHECK(!fitness3d.dominating(f1));
  BOOST_CHECK(!fitness4d.dominating(f1));
  BOOST_CHECK(fitness3d.dominating(fitness2d));
  BOOST_CHECK(fitness4d.dominating(fitness3d));
  BOOST_CHECK(!fitness2d.dominating(fitness3d));
  BOOST_CHECK(!fitness3d.dominating(fitness4d));
  BOOST_CHECK(!f1.dominating(f2));
  BOOST_CHECK(!f2.dominating(f1));
  BOOST_CHECK(f1.dominating(f1));
  BOOST_CHECK(empty.dominating(empty));
}

BOOST_AUTO_TEST_SUITE_END()
