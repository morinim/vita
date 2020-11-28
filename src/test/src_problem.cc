/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/problem.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

namespace vita::detail
{
template<class C>
std::set<std::vector<C>> seq_with_rep(const std::set<C> &, std::size_t);
}

TEST_SUITE("SRC_PROBLEM")
{

TEST_CASE("seq_with_rep")
{
  vita::category_t v0{1}, v1{2}, v2{3}, v3{4};

  std::set<vita::category_t> v = { v0 };
  auto seq(vita::detail::seq_with_rep(v, 1));
  std::set<vita::cvect> res = { {v0} };
  CHECK(seq.size() == 1);
  CHECK(seq == res);

  seq = vita::detail::seq_with_rep(v, 4);
  res = { {v0, v0, v0, v0} };
  CHECK(seq.size() == 1);
  CHECK(seq == res);

  v = {v0, v1};
  seq = vita::detail::seq_with_rep(v, 1);
  res = { {v0}, {v1} };
  CHECK(seq.size() == 2);
  CHECK(seq == res);

  v = {v0, v1};
  seq = vita::detail::seq_with_rep(v, 2);
  res = { {v0, v0}, {v0, v1}, {v1, v0}, {v1, v1} };
  CHECK(seq.size() == 4);
  CHECK(seq == res);

  v = {v0, v1, v2};
  seq = vita::detail::seq_with_rep(v, 3);
  res.clear();
  for (unsigned i(0); i < 3; ++i)
    for (unsigned j(0); j < 3; ++j)
      for (unsigned k(0); k < 3; ++k)
        res.insert({*std::next(v.begin(), i),
                    *std::next(v.begin(), j),
                    *std::next(v.begin(), k)});
  CHECK(seq.size() == 27);
  CHECK(seq == res);

  v = {v0, v1, v2, v3};
  seq = vita::detail::seq_with_rep(v, 8);
  CHECK(seq.size() ==
             v.size() * v.size() * v.size() * v.size() *
             v.size() * v.size() * v.size() * v.size());
}

TEST_CASE("Loading")
{
  vita::src_problem p("./test_resources/src_problem.xrff",
                      "./test_resources/src_problem.xml",
                      vita::typing::strong);

  CHECK(!!p);

  CHECK(p.data().size() == 3);
  CHECK(p.classification());
  CHECK(p.classes() == 3);
  CHECK(p.categories() == 4);
  CHECK(p.variables() == 3);
}

}  // TEST_SUITE("SRC_PROBLEM")
