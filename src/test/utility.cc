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

#include "utility/utility.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("UTILITY")
{

TEST_CASE("is_number")
{
  using namespace vita;

  CHECK(is_number("0"));
  CHECK(is_number("1"));
  CHECK(is_number("1.2"));
  CHECK(is_number("1."));
  CHECK(is_number(" 1.2"));
  CHECK(is_number(" 1.2 "));
  CHECK(!is_number("a"));
  CHECK(!is_number("a1"));
  CHECK(!is_number("1a"));
  CHECK(!is_number("1 a"));
  CHECK(!is_number("a 12"));
  CHECK(!is_number("11 12"));
  CHECK(!is_number("\"1\""));
  CHECK(!is_number("'1'"));
}

}  // TEST_SUITE("UTILITY")
