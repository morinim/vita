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

BOOST_AUTO_TEST_SUITE(test_src_problem)

BOOST_AUTO_TEST_CASE(t_seq_with_rep)
{
  vita::cvect v = {1};
  auto seq(vita::detail::seq_with_rep(v, 1));
  BOOST_REQUIRE_EQUAL(seq.size(), 1);
  BOOST_REQUIRE_EQUAL(seq[0][0], v[0]);

  seq = vita::detail::seq_with_rep(v, 4);
  vita::cvect r = {v[0], v[0], v[0], v[0]};
  BOOST_REQUIRE_EQUAL(seq.size(), 1);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(seq[0].begin(), seq[0].end(),
                                  r.begin(), r.end());

  v = {1, 2};
  seq = vita::detail::seq_with_rep(v, 1);
  BOOST_REQUIRE_EQUAL(seq.size(), 2);
  BOOST_REQUIRE_EQUAL(seq[0][0], v[0]);
  BOOST_REQUIRE_EQUAL(seq[1][0], v[1]);

  v = {1, 2};
  seq = vita::detail::seq_with_rep(v, 2);
  BOOST_REQUIRE_EQUAL(seq.size(), 4);
  std::vector<vita::cvect> rv{ {v[0], v[0]}, {v[0], v[1]}, {v[1], v[0]}, {v[1], v[1]} };

  for (unsigned i(0); i < seq.size(); ++i)
    BOOST_REQUIRE_EQUAL_COLLECTIONS(seq[i].begin(), seq[i].end(),
                                    rv[i].begin(), rv[i].end());

  v = {1, 2, 3};
  seq = vita::detail::seq_with_rep(v, 3);
  BOOST_REQUIRE_EQUAL(seq.size(), 27);
  for (unsigned i(0); i < 3; ++i)
    for (unsigned j(0); j < 3; ++j)
      for (unsigned k(0); k < 3; ++k)
      {
        const auto index(9*k + 3*j + i);
        r = {v[k], v[j], v[i]};

        BOOST_REQUIRE_EQUAL_COLLECTIONS(seq[index].begin(), seq[index].end(),
                                        r.begin(), r.end());
      }

  v = {1, 2, 3, 4};
  seq = vita::detail::seq_with_rep(v, 8);
  BOOST_REQUIRE_EQUAL(
    seq.size(),
    v.size()*v.size()*v.size()*v.size()*v.size()*v.size()*v.size()*v.size());
}

BOOST_AUTO_TEST_CASE(t_compatible)
{
  vita::src_problem p("src_problem.xrff", "", "src_problem.xml");

  BOOST_REQUIRE_EQUAL(!p, false);
}

BOOST_AUTO_TEST_SUITE_END()
