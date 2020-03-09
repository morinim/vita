/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>

#include "kernel/random.h"
#include "kernel/src/dataframe.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("DATAFRAME")
{

TEST_CASE("load_csv")
{
  using namespace vita;

  dataframe d1, d2, d3;
  constexpr std::size_t n1(10), n2(150), n3(351);

  CHECK(d1.read("./test_resources/mep.csv") == n1);
  CHECK(d1.size() == n1);
  CHECK(d2.read("./test_resources/iris.csv") == n2);
  CHECK(d2.size() == n2);
  CHECK(d3.read("./test_resources/ionosphere.csv") == n3);
  CHECK(d3.size() == n3);

  auto csv_filter = [](dataframe::record_t &)
  {
    return random::boolean();
  };

  std::size_t n(0), sup(10);
  for (unsigned i(0); i < sup; ++i)
  {
    dataframe d3f;

    d3f.read("./test_resources/ionosphere.csv", csv_filter);
    n += d3f.size();
  }

  const auto half(n3 * sup / 2);
  CHECK(9 * half <= 10 * n);
  CHECK(10 * n <= 11 * half);
}

}  // TEST_SUITE("DATAFRAME")
