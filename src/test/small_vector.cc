/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <iostream>

#include "kernel/staging/small_vector.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE small_vector_test
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

struct not_pod
{
  not_pod(int x = 10, double y = 3.1415) : a(x), b(y) {}

  bool operator!=(const not_pod &rhs) const
  {
    return a != rhs.a || std::fabs(b - rhs.b) > 0.00000001;
  }

  int    a;
  double b;
};

std::ostream &operator<<(std::ostream &o, const not_pod &x)
{
  return o << '(' << x.a << ", " << x.b << ')';
}

BOOST_AUTO_TEST_SUITE(small_vector)

BOOST_AUTO_TEST_CASE(t_constructor_assignment)
{
  vita::small_vector<double, 1> sv1;
  vita::small_vector<double, 1> sv1_bis(1, 3.1415);
  vita::small_vector<double, 2> sv2{1.0, 2.0};
  vita::small_vector<double, 3> sv3(3);

  BOOST_REQUIRE(sv1.empty());
  BOOST_REQUIRE_EQUAL(sv1.size(), 0);
  BOOST_REQUIRE(!sv1_bis.empty());
  BOOST_REQUIRE_EQUAL(sv1_bis.size(), 1);
  BOOST_REQUIRE(!sv2.empty());
  BOOST_REQUIRE_EQUAL(sv2.size(), 2);
  BOOST_REQUIRE(!sv3.empty());
  BOOST_REQUIRE_EQUAL(sv3.size(), 3);

  sv1 = sv1_bis;
  BOOST_REQUIRE_EQUAL_COLLECTIONS(sv1.begin(), sv1.end(),
                                  sv1_bis.begin(), sv1_bis.end());

  sv1 = {1.0, 2.0};
  BOOST_REQUIRE_EQUAL_COLLECTIONS(sv1.begin(), sv1.end(), sv2.begin(), sv2.end());

  sv3 = {1.0, 2.0, 3.0};
  sv1 = {1.0, 2.0, 3.0};
  BOOST_REQUIRE_EQUAL_COLLECTIONS(sv1.begin(), sv1.end(), sv3.begin(), sv3.end());

  sv1_bis = sv1;
  BOOST_REQUIRE_EQUAL_COLLECTIONS(sv1.begin(), sv1.end(),
                                  sv1_bis.begin(), sv1_bis.end());
}

BOOST_AUTO_TEST_CASE(t_push_back)
{
  vita::small_vector<double, 1> sv1;
  std::vector<double> v;

  for (unsigned i(0); i < 1000; ++i)
  {
    sv1.push_back(i);
    v.push_back(i);

    BOOST_REQUIRE_EQUAL_COLLECTIONS(sv1.begin(), sv1.end(), v.begin(), v.end());
  }

  vita::small_vector<int, 2> sv2;
  std::vector<int> v2;

  for (unsigned i(0); i < 1000; ++i)
  {
    sv2.push_back(static_cast<int>(i));
    v2.push_back(static_cast<int>(i));

    BOOST_CHECK_EQUAL_COLLECTIONS(sv2.begin(), sv2.end(), v2.begin(), v2.end());
  }

  assert(!std::is_pod<not_pod>::value);

  vita::small_vector<not_pod, 3> sv3;
  std::vector<not_pod> v3;

  for (unsigned i(0); i < 1000; ++i)
  {
    sv3.push_back(not_pod(static_cast<int>(i), i + 1));
    v3.push_back(not_pod(static_cast<int>(i), i + 1));

    BOOST_CHECK_EQUAL_COLLECTIONS(sv3.begin(), sv3.end(), v3.begin(), v3.end());
  }
}

BOOST_AUTO_TEST_CASE(t_insert)
{
  BOOST_TEST_CHECKPOINT("INSERT AT THE END (APPEND)");
  {
    std::vector<unsigned> src1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    vita::small_vector<unsigned, 1> sv1;
    std::vector<unsigned> v1;

    for (unsigned i(0); i < 200; ++i)
    {
      sv1.insert(sv1.end(), src1.begin(), src1.end());
      v1.insert(v1.end(), src1.begin(), src1.end());

      BOOST_CHECK_EQUAL_COLLECTIONS(sv1.begin(), sv1.end(),
                                    v1.begin(), v1.end());
    }
  }

  BOOST_TEST_CHECKPOINT("INSERT AT THE BEGINNING");
  {
    std::vector<short> src2 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    vita::small_vector<short, 2> sv2;
    std::vector<short> v2;

    for (unsigned i(0); i < 200; ++i)
    {
      sv2.insert(sv2.begin(), src2.begin(), src2.end());
      v2.insert(v2.begin(), src2.begin(), src2.end());

      BOOST_CHECK_EQUAL_COLLECTIONS(sv2.begin(), sv2.end(),
                                    v2.begin(), v2.end());
    }
  }

  BOOST_TEST_CHECKPOINT("INSERT IN THE MIDDLE");
  {
    std::vector<not_pod> src3 = {not_pod(0, 1.0), not_pod(2, 3.0),
                                 not_pod(4, 5.0), not_pod(6, 7.0),
                                 not_pod(8, 9.0), not_pod(0, 0.0)};
    vita::small_vector<not_pod, 3> sv3;
    std::vector<not_pod> v3;

    for (unsigned i(0); i < 200; ++i)
    {
      const auto n(static_cast<std::ptrdiff_t>(v3.size() / 2));

      sv3.insert(std::next(sv3.begin(), n), src3.begin(), src3.end());
      v3.insert(std::next(v3.begin(), n), src3.begin(), src3.end());

      BOOST_CHECK_EQUAL_COLLECTIONS(sv3.begin(), sv3.end(),
                                    v3.begin(), v3.end());
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
