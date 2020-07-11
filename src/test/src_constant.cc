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
  using namespace vita;

  const bool b(true);
  constant<bool> bc(b);
  CHECK(std::get<D_INT>(bc.eval(nullptr)) == b);
  CHECK(bc.is_valid());

  const int i(1234);
  constant<int> ic(i);
  CHECK(std::get<D_INT>(ic.eval(nullptr)) == i);
  CHECK(ic.is_valid());

  const double d(3.14);
  constant<double> dc(d);
  CHECK(std::get<D_DOUBLE>(dc.eval(nullptr)) == doctest::Approx(d));
  CHECK(dc.is_valid());

  const std::string s("A STRING CONSTANT");
  constant<std::string> sc(s);
  CHECK(std::get<D_STRING>(sc.eval(nullptr)) == s);
  CHECK(sc.is_valid());
}

}  // TEST_SUITE("SRC_CONSTANT")
