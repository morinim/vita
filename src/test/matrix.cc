/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017 EOS di Manlio Morini.
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

BOOST_AUTO_TEST_CASE(MatrixConstructor)
{
  vita::matrix<int> m(3, 4);

  BOOST_TEST(m.rows() == 3);
  BOOST_TEST(m.cols() == 4);
  BOOST_TEST(!m.empty());
  BOOST_TEST(m(0, 0) == decltype(m)::value_type());

  m = { {1, 2, 3},
        {4, 5, 6} };

  BOOST_TEST(m.rows() == 2);
  BOOST_TEST(m.cols() == 3);
  BOOST_TEST(!m.empty());
  BOOST_TEST(m(0, 0) == 1);
}

BOOST_AUTO_TEST_CASE(EmptyMatrix)
{
  vita::matrix<int> m;

  BOOST_TEST(m.empty());
  BOOST_TEST(m.cols() == 0);

  decltype(m) m1(3, 3);
  BOOST_TEST(!m1.empty());

  std::stringstream ss;
  BOOST_TEST(m.save(ss));

  BOOST_TEST(m1.load(ss));
  BOOST_TEST(m == m1);
}

BOOST_AUTO_TEST_CASE(MatrixFliplr)
{
  vita::matrix<int> m = { {1, 2, 3},
                          {4, 5, 6} };

  vita::matrix<int> f = { {3, 2, 1},
                          {6, 5, 4} };

  BOOST_TEST(f == vita::fliplr(m));

  m = { {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 0, 1, 2} };

  f = { {4, 3, 2, 1},
        {8, 7, 6, 5},
        {2, 1, 0, 9} };

  BOOST_TEST(f == vita::fliplr(m));
}

BOOST_AUTO_TEST_CASE(MatrixFlipud)
{
  vita::matrix<int> m = { {1, 2, 3},
                          {4, 5, 6} };

  vita::matrix<int> f = { {4, 5, 6},
                          {1, 2, 3} };

  BOOST_TEST(f == vita::flipud(m));

  m = { {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 0, 1, 2} };

  f = { {9, 0, 1, 2},
        {5, 6, 7, 8},
        {1, 2, 3, 4} };

  BOOST_TEST(f == vita::flipud(m));
}

BOOST_AUTO_TEST_CASE(MatrixLessThan)
{
  vita::matrix<int> m0 = { {9} };

  vita::matrix<int> m1 = { {9, 9},
                           {9, 9} };

  vita::matrix<int> m2 = { {1, 2, 3},
                           {4, 5, 6} };

  vita::matrix<int> m3 = { {1, 2, 3},
                           {4, 5, 9} };

  vita::matrix<int> empty;

  BOOST_TEST(m0 < m1);
  BOOST_TEST(m2 < m0);
  BOOST_TEST(m3 < m0);
  BOOST_TEST(m2 < m1);
  BOOST_TEST(m3 < m1);
  BOOST_TEST(m2 < m3);
  BOOST_TEST(empty < m0);
  BOOST_TEST(empty < m1);
  BOOST_TEST(empty < m2);
  BOOST_TEST(empty < m3);
}

BOOST_AUTO_TEST_CASE(MatrixSerialization)
{
  vita::matrix<int> m(100, 100);

  for (unsigned i(0); i < 1000; ++i)
  {
    for (auto &elem : m)
      elem = vita::random::between(0, 1000);

    std::stringstream ss;
    BOOST_TEST(m.save(ss));

    decltype(m) m1;

    BOOST_TEST(m1.load(ss));
    BOOST_TEST(m == m1);
  }
}

BOOST_AUTO_TEST_SUITE_END()
