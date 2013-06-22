/**
 *
 *  \file test_primitive.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>

#include "random.h"
#include "timer.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE symbol_set
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture3.h"
#endif

BOOST_FIXTURE_TEST_SUITE(symbol_set, F_FACTORY3)

BOOST_AUTO_TEST_CASE(Speed)
{
  const vita::symbol_set &ss(env.sset);

  const unsigned n(10000000);
  vita::timer t;
  for (unsigned i(0); i < n; ++i)
    ss.roulette();

  BOOST_TEST_MESSAGE(static_cast<unsigned>(1000.0 * n / t.elapsed())
                     << " extractions/sec");
}

BOOST_AUTO_TEST_SUITE_END()
