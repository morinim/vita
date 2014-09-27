/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <sstream>

#include "kernel/evolution.h"
#include "kernel/i_mep.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE summary
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_FIXTURE_TEST_SUITE(summary, F_FACTORY1)

BOOST_AUTO_TEST_CASE(Serialization)
{
  env.code_length = 100;

  for (unsigned i(0); i < 2000; ++i)
  {
    vita::summary<vita::i_mep> before;

    before.elapsed = vita::random::between(10u, 10000u);
    before.mutations = vita::random::between(100u, 100000u);
    before.crossovers = vita::random::between(100u, 100000u);
    before.gen = vita::random::between(0u, 1000u);
    before.last_imp = vita::random::between(0u, 1000u);

    if (vita::random::boolean(0.8))
      before.best = {
        vita::i_mep(env, sset),
        vita::fitness_t(vita::random::between<double>(0.0, 1000.0))
      };

    std::stringstream ss;
    BOOST_REQUIRE(before.save(ss));

    vita::summary<vita::i_mep> after;
    BOOST_REQUIRE(after.load(ss, env, sset));

    BOOST_CHECK_EQUAL(before.elapsed, after.elapsed);
    BOOST_CHECK_EQUAL(before.mutations, after.mutations);
    BOOST_CHECK_EQUAL(before.crossovers, after.crossovers);
    BOOST_CHECK_EQUAL(before.gen, after.gen);
    BOOST_CHECK_EQUAL(before.last_imp, after.last_imp);

    if (before.best)
    {
      BOOST_CHECK_EQUAL(before.best->ind, after.best->ind);
      BOOST_CHECK_EQUAL(before.best->fitness, after.best->fitness);
    }
    else
      BOOST_CHECK(!after.best);
  }
}

BOOST_AUTO_TEST_SUITE_END()
