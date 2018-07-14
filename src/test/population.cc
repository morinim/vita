/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/i_mep.h"
#include "kernel/population.h"

#include "test/fixture1.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("POPULATION")
{

TEST_CASE_FIXTURE(fixture1, "Creation")
{
  prob.env.layers = 1;

  for (unsigned i(0); i < 100; ++i)
  {
    prob.env.individuals = vita::random::between(30u, 200u);

    vita::population<vita::i_mep> pop(prob);

    CHECK(prob.env.individuals == pop.individuals());
    CHECK(pop.debug());
  }
}

TEST_CASE_FIXTURE(fixture1, "Layers and individuals")
{
  for (unsigned i(0); i < 100; ++i)
  {
    prob.env.individuals = vita::random::between(30u, 200u);
    prob.env.layers = vita::random::between(1u, 10u);

    vita::population<vita::i_mep> pop(prob);

    for (unsigned l(0); l < pop.layers(); ++l)
    {
      const auto n(vita::random::between(0u, pop.individuals(l)));

      const auto before(pop.individuals(l));

      for (unsigned j(0); j < n; ++j)
        pop.pop_from_layer(l);

      CHECK(pop.individuals(l) == before - n);
    }

    unsigned count(std::accumulate(pop.begin(), pop.end(), 0,
                                   [](auto acc, auto) { return ++acc; }));

    CHECK(count == pop.individuals());
  }
}

TEST_CASE_FIXTURE(fixture1, "Serialization")
{
  using namespace vita;

  for (unsigned i(0); i < 100; ++i)
  {
    prob.env.individuals = random::between(30u, 300u);
    prob.env.tournament_size = random::between<unsigned>(1,prob.env.mate_zone);

    std::stringstream ss;
    vita::population<i_mep> pop1(prob);

    CHECK(pop1.save(ss));

    decltype(pop1) pop2(prob);
    CHECK(pop2.load(ss, prob));
    CHECK(pop2.debug());

    CHECK(pop1.layers() == pop2.layers());
    CHECK(pop1.individuals() == pop2.individuals());
    for (unsigned l(0); l < pop1.layers(); ++l)
    {
      CHECK(pop1.individuals(l) == pop2.individuals(l));

      for (unsigned j(0); j < pop1.individuals(); ++j)
      {
        const vita::population<i_mep>::coord c{l, j};
        CHECK(pop1[c] == pop2[c]);
      }
    }
  }
}

}  // TEST_SUITE("POPULATION")
