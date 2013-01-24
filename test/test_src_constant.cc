/**
 *
 *  \file test_src_constant.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <sstream>

#include "src_constant.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE src_constant
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(src_constant)

BOOST_AUTO_TEST_CASE(Base)
{
  const bool b(true);
  vita::constant<bool> bc(b);
  BOOST_CHECK_EQUAL(vita::any_cast<bool>(bc.eval(0)), b);
  BOOST_CHECK(bc.check());

  const int i(1234);
  vita::constant<int> ic(i);
  BOOST_CHECK_EQUAL(vita::any_cast<int>(ic.eval(0)), i);
  BOOST_CHECK(ic.check());

  const double d(3.14);
  vita::constant<double> dc(d);
  BOOST_CHECK_EQUAL(vita::any_cast<double>(dc.eval(0)), d);
  BOOST_CHECK(dc.check());

  const std::string s("A STRING CONSTANT");
  vita::constant<std::string> sc(s);
  BOOST_CHECK_EQUAL(vita::any_cast<std::string>(sc.eval(0)), s);
  BOOST_CHECK(sc.check());
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  vita::constant<int> t("1234");

  std::stringstream stream;
  BOOST_REQUIRE(t.save(stream));

  vita::constant<int> t1("5678");
  stream.seekg(0, std::ios::beg);
  BOOST_REQUIRE(t1.load(stream));

  BOOST_CHECK(t1.check());
  BOOST_CHECK_EQUAL(t1.display(), t.display());
  BOOST_CHECK_EQUAL(vita::any_cast<int>(t.eval(0)),
                    vita::any_cast<int>(t1.eval(0)));
}

BOOST_AUTO_TEST_SUITE_END()
