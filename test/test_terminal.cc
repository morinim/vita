/**
 *
 *  \file test_terminal.cc
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
#define BOOST_TEST_MODULE terminal
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(terminal)

BOOST_AUTO_TEST_CASE(Base)
{
  vita::constant<std::string> t("A TERMINAL");

  BOOST_CHECK_EQUAL(t.associative(), false);
  BOOST_CHECK_EQUAL(t.arity(), 0);
  BOOST_CHECK_EQUAL(t.auto_defined(), false);
  BOOST_CHECK(t.check());
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  vita::constant<int> t("1234");

  std::stringstream stream;
  BOOST_REQUIRE(t.save(stream));

  vita::constant<int> t1("5678");
  stream.seekg(0, std::ios::beg);
  BOOST_REQUIRE(t1.load(stream));

  BOOST_CHECK_EQUAL(t1.associative(), t.associative());
  BOOST_CHECK_EQUAL(t1.arity(), t.arity());
  BOOST_CHECK_EQUAL(t1.auto_defined(), t.auto_defined());
  BOOST_CHECK(t1.check());
  BOOST_CHECK_EQUAL(t1.display(), t.display());
}

BOOST_AUTO_TEST_SUITE_END()
