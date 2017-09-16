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

#include "kernel/random.h"
#include "kernel/src/data.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE lambda
#include <boost/test/unit_test.hpp>

using namespace boost;

#endif

BOOST_AUTO_TEST_SUITE(t_data)

BOOST_AUTO_TEST_CASE(t_load_csv)
{
  using namespace vita;

  src_data d1, d2, d3;
  const std::size_t n1(10), n2(150), n3(351);

  BOOST_TEST(d1.load("./test_resources/mep.csv") == n1);
  BOOST_TEST(d1.size() == n1);
  BOOST_TEST(d2.load("./test_resources/iris.csv") == n2);
  BOOST_TEST(d2.size() == n2);
  BOOST_TEST(d3.load("./test_resources/ionosphere.csv") = n3);
  BOOST_TEST(d3.size() == n3);

  auto csv_filter = [](src_data::record_t &)
  {
    return random::boolean();
  };

  std::size_t n(0), sup(10);
  for (unsigned i(0); i < sup; ++i)
  {
    src_data d3f;

    d3f.load("./test_resources/ionosphere.csv", csv_filter);
    n += d3f.size();
  }

  const auto half(n3 * sup / 2);
  BOOST_TEST(9 * half <= 10 * n);
  BOOST_TEST(10 * n <= 11 * half);
}

BOOST_AUTO_TEST_SUITE_END()
