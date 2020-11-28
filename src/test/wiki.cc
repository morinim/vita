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

#include <sstream>

#include "kernel/gp/src/category_set.h"
#include "kernel/gp/src/dataframe.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("WIKI_DATAFRAME")
{

struct fixture1
{
  fixture1() : dataset(R"( A,   B, C,  D
                          a0, 0.0, 0, d0
                          a1, 0.1, 1, d1
                          a2, 0.2, 2, d2)"),
               d()
  {
  }

  std::istringstream dataset;
  vita::dataframe d;
};

TEST_CASE_FIXTURE(fixture1, "dataframe import data 1")
{
  using namespace vita;

  d.read_csv(dataset);

  CHECK(d.columns[0].name == "A");
  CHECK(d.columns[1].name == "B");
  CHECK(d.columns[2].name == "C");
  CHECK(d.columns[3].name == "D");
}

TEST_CASE_FIXTURE(fixture1, "dataframe import data 2")
{
  using namespace vita;

  d.read_csv(dataset, dataframe::params().output(2));

  CHECK(d.columns[0].name == "C");
  CHECK(d.columns[1].name == "A");
  CHECK(d.columns[2].name == "B");
  CHECK(d.columns[3].name == "D");

  std::cout << "Label of the first example is: "
            << lexical_cast<double>(d.front().output)
            << "\nFeatures are:"
            << "\nA: " << lexical_cast<std::string>(d.front().input[0])
            << "\nB: " << lexical_cast<double>(     d.front().input[1])
            << "\nD: " << lexical_cast<std::string>(d.front().input[2]) << '\n';
}

TEST_CASE_FIXTURE(fixture1, "dataframe import data 3")
{
  using namespace vita;

  d.read_csv(dataset, dataframe::params().no_output());

  CHECK(d.columns[0].name == "");
  CHECK(d.columns[1].name == "A");
  CHECK(d.columns[2].name == "B");
  CHECK(d.columns[3].name == "C");
  CHECK(d.columns[4].name == "D");

  CHECK(!has_value(d.front().output));
}

TEST_CASE_FIXTURE(fixture1, "dataframe columns")
{
  using namespace vita;

  d.read_csv(dataset, dataframe::params().header());

  std::cout << "Name of the first column: " << d.columns[0].name
            << "\nDomain of the first column: " << d.columns[0].domain;

  std::cout << "\nThere are " << d.columns.size() << " columns\n";
}

}  // TEST_SUITE("WIKI_DATAFRAME")
