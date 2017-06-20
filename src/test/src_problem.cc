/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017 EOS di Manlio Morini.
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
template<class C>
std::set<std::vector<C>> seq_with_rep(const std::set<C> &, std::size_t);
}}

BOOST_AUTO_TEST_SUITE(test_src_problem)

BOOST_AUTO_TEST_CASE(t_seq_with_rep)
{
  vita::category_t v0{1}, v1{2}, v2{3}, v3{4};

  std::set<vita::category_t> v = { v0 };
  auto seq(vita::detail::seq_with_rep(v, 1));
  std::set<vita::cvect> res = { {v0} };
  BOOST_TEST(seq.size() == 1);
  BOOST_TEST(seq == res);

  seq = vita::detail::seq_with_rep(v, 4);
  res = { {v0, v0, v0, v0} };
  BOOST_TEST(seq.size() == 1);
  BOOST_TEST(seq == res);

  v = {v0, v1};
  seq = vita::detail::seq_with_rep(v, 1);
  res = { {v0}, {v1} };
  BOOST_TEST(seq.size() == 2);
  BOOST_TEST(seq == res);

  v = {v0, v1};
  seq = vita::detail::seq_with_rep(v, 2);
  res = { {v0, v0}, {v0, v1}, {v1, v0}, {v1, v1} };
  BOOST_TEST(seq.size() == 4);
  BOOST_TEST(seq == res);

  v = {v0, v1, v2};
  seq = vita::detail::seq_with_rep(v, 3);
  res.clear();
  for (unsigned i(0); i < 3; ++i)
    for (unsigned j(0); j < 3; ++j)
      for (unsigned k(0); k < 3; ++k)
        res.insert({*std::next(v.begin(), i),
                    *std::next(v.begin(), j),
                    *std::next(v.begin(), k)});
  BOOST_TEST(seq.size() == 27);
  BOOST_TEST(seq == res);

  v = {v0, v1, v2, v3};
  seq = vita::detail::seq_with_rep(v, 8);
  BOOST_TEST(seq.size() ==
             v.size() * v.size() * v.size() * v.size() *
             v.size() * v.size() * v.size() * v.size());
}

BOOST_AUTO_TEST_CASE(t_loading)
{
  vita::src_problem p("../test/src_problem.xrff", "",
                      "../test/src_problem.xml");

  BOOST_REQUIRE(!!p);

  BOOST_REQUIRE_EQUAL(p.data()->size(), 3);
  BOOST_REQUIRE(p.classification());
  BOOST_REQUIRE_EQUAL(p.classes(), 3);
  BOOST_REQUIRE_EQUAL(p.categories(), 4);
  BOOST_REQUIRE_EQUAL(p.variables(), 3);
}

BOOST_AUTO_TEST_SUITE_END()
