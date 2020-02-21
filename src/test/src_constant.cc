/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/src/constant.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("SRC_CONSTANT")
{

TEST_CASE("Base")
{
  const bool b(true);
  vita::constant<bool> bc(b);
  CHECK(std::any_cast<bool>(bc.eval(nullptr)) == b);
  CHECK(bc.debug());

  const int i(1234);
  vita::constant<int> ic(i);
  CHECK(std::any_cast<int>(ic.eval(nullptr)) == i);
  CHECK(ic.debug());

  const double d(3.14);
  vita::constant<double> dc(d);
  CHECK(std::any_cast<double>(dc.eval(nullptr)) == doctest::Approx(d));
  CHECK(dc.debug());

  const std::string s("A STRING CONSTANT");
  vita::constant<std::string> sc(s);
  CHECK(std::any_cast<std::string>(sc.eval(nullptr)) == s);
  CHECK(sc.debug());
}

}  // TEST_SUITE("SRC_CONSTANT")
