/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2022 EOS di Manlio Morini.
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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("MATRIX")
{

TEST_CASE("Constructor")
{
  vita::matrix<int> m(3, 4);

  CHECK(m.rows() == 3);
  CHECK(m.cols() == 4);
  CHECK(!m.empty());
  CHECK(m(0, 0) == vita::matrix<int>::value_type());

  m = { {1, 2, 3},
        {4, 5, 6} };

  CHECK(m.rows() == 2);
  CHECK(m.cols() == 3);
  CHECK(!m.empty());
  CHECK(m(0, 0) == 1);
}

TEST_CASE("Empty matrix")
{
  vita::matrix<int> m;

  CHECK(m.empty());
  CHECK(m.cols() == 0);

  decltype(m) m1(3, 3);
  CHECK(!m1.empty());

  std::stringstream ss;
  CHECK(m.save(ss));

  CHECK(m1.load(ss));
  CHECK(m == m1);
}

TEST_CASE("Fliplr")
{
  vita::matrix<int> m = { {1, 2, 3},
                          {4, 5, 6} };

  vita::matrix<int> f = { {3, 2, 1},
                          {6, 5, 4} };

  CHECK(f == vita::fliplr(m));

  m = { {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 0, 1, 2} };

  f = { {4, 3, 2, 1},
        {8, 7, 6, 5},
        {2, 1, 0, 9} };

  CHECK(f == vita::fliplr(m));
}

TEST_CASE("Flipud")
{
  vita::matrix<int> m = { {1, 2, 3},
                          {4, 5, 6} };

  vita::matrix<int> f = { {4, 5, 6},
                          {1, 2, 3} };

  CHECK(f == vita::flipud(m));

  m = { {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 0, 1, 2} };

  f = { {9, 0, 1, 2},
        {5, 6, 7, 8},
        {1, 2, 3, 4} };

  CHECK(f == vita::flipud(m));
}

TEST_CASE("Less than")
{
  vita::matrix<int> m0 = { {9} };

  vita::matrix<int> m1 = { {9, 9},
                           {9, 9} };

  vita::matrix<int> m2 = { {1, 2, 3},
                           {4, 5, 6} };

  vita::matrix<int> m3 = { {1, 2, 3},
                           {4, 5, 9} };

  vita::matrix<int> empty;

  CHECK(m0 < m1);
  CHECK(m2 < m0);
  CHECK(m3 < m0);
  CHECK(m2 < m1);
  CHECK(m3 < m1);
  CHECK(m2 < m3);
  CHECK(empty < m0);
  CHECK(empty < m1);
  CHECK(empty < m2);
  CHECK(empty < m3);
}

TEST_CASE("Transpose")
{
  vita::matrix<int> m = { {1, 2, 3},
                          {4, 5, 6} };

  vita::matrix<int> t = { {1, 4},
                          {2, 5},
                          {3, 6} };

  CHECK(t == vita::transpose(m));

  m = { {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 0, 1, 2} };

  t = { {1, 5, 9},
        {2, 6, 0},
        {3, 7, 1},
        {4, 8, 2} };

  CHECK(t == vita::transpose(m));

  m = { {1} };
  t = { {1} };

  CHECK(t == vita::transpose(m));
}

TEST_CASE("Rotation")
{
  const vita::matrix<int> m = { {1,  6, 11, 16, 21},
                                {2,  7, 12, 17, 22},
                                {3,  8, 13, 18, 23},
                                {4,  9, 14, 19, 24},
                                {5, 10, 15, 20, 25} };

  const vita::matrix<int> r90 = { {21, 22, 23, 24, 25},
                                  {16, 17, 18, 19, 20},
                                  {11, 12, 13, 14, 15},
                                  { 6,  7,  8,  9, 10},
                                  { 1,  2,  3,  4,  5} };

  const vita::matrix<int> r180 = { {25, 20, 15, 10, 5},
                                   {24, 19, 14,  9, 4},
                                   {23, 18, 13,  8, 3},
                                   {22, 17, 12,  7, 2},
                                   {21, 16, 11,  6, 1} };

  const vita::matrix<int> r270 = { { 5,  4,  3,  2,  1},
                                   {10,  9,  8,  7,  6},
                                   {15, 14, 13, 12, 11},
                                   {20, 19, 18, 17, 16},
                                   {25, 24, 23, 22, 21} };

  CHECK(   m == vita::rot90(   m, 0));
  CHECK( r90 == vita::rot90( r90, 0));
  CHECK(r180 == vita::rot90(r180, 0));
  CHECK(r270 == vita::rot90(r270, 0));

  CHECK( r90 == vita::rot90(   m, 1));

  CHECK(r180 == vita::rot90( r90, 1));
  CHECK(r180 == vita::rot90(   m, 2));

  CHECK(r270 == vita::rot90(r180, 1));
  CHECK(r270 == vita::rot90( r90, 2));
  CHECK(r270 == vita::rot90(   m, 3));
}

TEST_CASE("Serialization")
{
  vita::matrix<int> m(100, 100);

  for (unsigned i(0); i < 1000; ++i)
  {
    for (auto &elem : m)
      elem = vita::random::between(0, 1000);

    std::stringstream ss;
    CHECK(m.save(ss));

    decltype(m) m1;

    CHECK(m1.load(ss));
    CHECK(m == m1);
  }
}

}  // TEST_SUITE("MATRIX")
