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

#include "kernel/src/constant.h"

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
  BOOST_CHECK(t.debug());
}

BOOST_AUTO_TEST_SUITE_END()
