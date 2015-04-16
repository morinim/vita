/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/src/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE test_src_problem
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

namespace vita { namespace detail {
template<class C> std::vector<C> seq_with_rep(const C &, std::size_t);
}}

template<class T, class U>
bool equal(const T &c1, const U &c2)
{
  assert(c1.size() == c2.size());
  return std::equal(c1.begin(), c1.end(), c2.begin());
}

BOOST_AUTO_TEST_SUITE(test_src_problem)

BOOST_AUTO_TEST_CASE(t_seq_with_rep)
{
  vita::cvect v = {1};
  auto seq(vita::detail::seq_with_rep(v, 1));
  BOOST_REQUIRE_EQUAL(seq.size(), 1);
  BOOST_REQUIRE_EQUAL(seq[0][0], v[0]);

  seq = vita::detail::seq_with_rep(v, 4);
  BOOST_REQUIRE_EQUAL(seq.size(), 1);
  BOOST_REQUIRE(equal(seq[0], vita::cvect{v[0], v[0], v[0], v[0]}));

  v = {1, 2};
  seq = vita::detail::seq_with_rep(v, 1);
  BOOST_REQUIRE_EQUAL(seq.size(), 2);
  BOOST_REQUIRE_EQUAL(seq[0][0], v[0]);
  BOOST_REQUIRE_EQUAL(seq[1][0], v[1]);

  v = {1, 2};
  seq = vita::detail::seq_with_rep(v, 2);
  BOOST_REQUIRE_EQUAL(seq.size(), 4);
  BOOST_REQUIRE(equal(seq[0], vita::cvect{v[0], v[0]}));
  BOOST_REQUIRE(equal(seq[1], vita::cvect{v[0], v[1]}));
  BOOST_REQUIRE(equal(seq[2], vita::cvect{v[1], v[0]}));
  BOOST_REQUIRE(equal(seq[3], vita::cvect{v[1], v[1]}));

  v = {1, 2, 3};
  seq = vita::detail::seq_with_rep(v, 3);
  BOOST_REQUIRE_EQUAL(seq.size(), 27);
  for (unsigned i(0); i < 3; ++i)
    for (unsigned j(0); j < 3; ++j)
      for (unsigned k(0); k < 3; ++k)
        BOOST_REQUIRE(equal(seq[9*k + 3*j + i],
                            vita::cvect{v[k], v[j], v[i]}));

  v = {1, 2, 3, 4};
  seq = vita::detail::seq_with_rep(v, 8);
  BOOST_REQUIRE_EQUAL(
    seq.size(),
    v.size()*v.size()*v.size()*v.size()*v.size()*v.size()*v.size()*v.size());
}

BOOST_AUTO_TEST_SUITE_END()
