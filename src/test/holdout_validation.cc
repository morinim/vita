/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/holdout_validation.h"
#include "kernel/gp/src/problem.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

bool near_integers(std::size_t x, std::size_t y)
{
  return x == y
         || (x + 1 == y && x + 1 != 0)
         || (y + 1 == x && y + 1 != 0);
}

TEST_SUITE("HOLDOUT_VALIDATION")
{

TEST_CASE("Timing")
{
  using namespace vita;

  src_problem p("./test_resources/iris.csv");
  CHECK(!!p);

  const auto examples(p.data().size());

  holdout_validation v(p);

  // Partitioning is performed at run 0 only.
  p.env.validation_percentage = 50;
  v.init(1);

  CHECK(p.data(dataset_t::training).size() == examples);
  CHECK(p.data(dataset_t::validation).empty());
}

TEST_CASE("Cardinality")
{
  using namespace vita;

  src_problem p("./test_resources/iris.csv");
  CHECK(!!p);

  const dataframe orig(p.data());
  const auto examples(p.data().size());

  holdout_validation v(p);

  for (unsigned perc(0); perc < 100; ++perc)
  {
    p.data(dataset_t::training) = orig;
    p.data(dataset_t::validation).clear();
    p.env.validation_percentage = perc;

    v.init(0);

    CHECK(near_integers(p.data(dataset_t::validation).size(),
                        examples * perc / 100));
    CHECK(p.data(dataset_t::training).size()
          + p.data(dataset_t::validation).size() == examples);
  }
}

TEST_CASE("Probabilities")
{
  using namespace vita;

  src_problem p("./test_resources/iris.csv");
  CHECK(!!p);

  // Output value changed to be used as unique key for example identification.
  int i(0);
  for (auto &e : p.data())
    e.output = i++;

  const auto examples(p.data().size());
  const dataframe orig(p.data());

  holdout_validation v(p);

  p.env.validation_percentage = 33;

  std::vector<unsigned> count(examples);

  const std::size_t extractions(10000);
  for (unsigned j(0); j < extractions; ++j)
  {
    p.data(dataset_t::training) = orig;
    p.data(dataset_t::validation).clear();

    v.init(0);

    for (const auto &e : p.data(dataset_t::validation))
      ++count[std::get<int>(e.output)];
  }

  const auto expected(extractions * *p.env.validation_percentage / 100);
  const auto tolerance_perc(10);
  const auto tolerance_inf(expected * (100 - tolerance_perc) / 100);
  const auto tolerance_sup(expected * (100 + tolerance_perc) / 100);

  for (auto x : count)
  {
    CHECK(x > tolerance_inf);
    CHECK(x < tolerance_sup);
  }
}

}  // TEST_SUITE("HOLDOUT_VALIDATION")
