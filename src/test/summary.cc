/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
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
  using vita::random::between;

  prob.env.mep.code_length = 100;

  for (unsigned i(0); i < 2000; ++i)
  {
    vita::summary<vita::i_mep> before;

    before.elapsed = std::chrono::milliseconds(between(10u, 10000u));
    before.mutations = between(100u, 100000u);
    before.crossovers = between(100u, 100000u);
    before.gen = between(0u, 1000u);
    before.last_imp = between(0u, 1000u);

    if (vita::random::boolean(0.8))
    {
      before.best.solution = vita::i_mep(prob);
      before.best.score.fitness = {between(0.0, 1000.0)};
      before.best.score.accuracy = between(0.0, 1.0);
    }

    std::stringstream ss;
    BOOST_REQUIRE(before.save(ss));

    vita::summary<vita::i_mep> after;
    BOOST_REQUIRE(after.load(ss, prob));

    BOOST_CHECK_EQUAL(before.elapsed.count(), after.elapsed.count());
    BOOST_CHECK_EQUAL(before.mutations, after.mutations);
    BOOST_CHECK_EQUAL(before.crossovers, after.crossovers);
    BOOST_CHECK_EQUAL(before.gen, after.gen);
    BOOST_CHECK_EQUAL(before.last_imp, after.last_imp);

    BOOST_CHECK_EQUAL(before.best.solution, after.best.solution);
    BOOST_CHECK_EQUAL(before.best.score.fitness, after.best.score.fitness);
    BOOST_CHECK_EQUAL(before.best.score.accuracy, after.best.score.accuracy);
  }
}

BOOST_AUTO_TEST_SUITE_END()
