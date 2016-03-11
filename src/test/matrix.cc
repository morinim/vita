/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/random.h"
#include "utility/matrix.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE Matrix
#include <boost/test/unit_test.hpp>

using namespace boost;

#endif


BOOST_AUTO_TEST_SUITE(matrix)

BOOST_AUTO_TEST_CASE(EmptyMatrix)
{
  vita::matrix<int> m;

  BOOST_REQUIRE(m.empty());
  BOOST_REQUIRE_EQUAL(m.cols(), 0);

  decltype(m) m1(3, 3);
  BOOST_REQUIRE(!m1.empty());

  std::stringstream ss;
  BOOST_REQUIRE(m.save(ss));

  BOOST_REQUIRE(m1.load(ss));
  BOOST_REQUIRE_EQUAL(m, m1);
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  vita::matrix<int> m(100, 100);

  for (unsigned i(0); i < 1000; ++i)
  {
    for (auto &elem : m)
      elem = vita::random::between(0, 1000);

    std::stringstream ss;
    BOOST_REQUIRE(m.save(ss));

    decltype(m) m1;

    BOOST_REQUIRE(m1.load(ss));
    BOOST_REQUIRE_EQUAL(m, m1);
  }
}

BOOST_AUTO_TEST_SUITE_END()
