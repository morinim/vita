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

#include <sstream>

#include "kernel/evolution.h"
#include "kernel/gp/mep/i_mep.h"

#include "test/fixture1.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("SUMMARY")
{

TEST_CASE_FIXTURE(fixture1, "Serialization")
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
    CHECK(before.save(ss));

    vita::summary<vita::i_mep> after;
    CHECK(after.load(ss, prob));

    CHECK(before.elapsed.count() == after.elapsed.count());
    CHECK(before.mutations == after.mutations);
    CHECK(before.crossovers == after.crossovers);
    CHECK(before.gen == after.gen);
    CHECK(before.last_imp == after.last_imp);

    CHECK(before.best.solution == after.best.solution);
    CHECK(before.best.score.fitness == after.best.score.fitness);
    CHECK(before.best.score.accuracy
          == doctest::Approx(after.best.score.accuracy));
  }
}

}  // TEST_SUITE("SUMMARY")
