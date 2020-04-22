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

#include "kernel/value.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("VALUE_T")
{

TEST_CASE("basic_value_t")
{
  using namespace vita;

  value_t v;
  CHECK(!has_value(v));
  CHECK(std::holds_alternative<std::monostate>(v));

  v = 3.14;
  CHECK(has_value(v));
  CHECK(std::holds_alternative<D_DOUBLE>(v));

  v = 12;
  CHECK(has_value(v));
  CHECK(std::holds_alternative<D_INT>(v));

  std::ostringstream ss;
  ss << v;
  CHECK(ss.str() == "12");
}

}  // TEST_SUITE("VALUE_T")
