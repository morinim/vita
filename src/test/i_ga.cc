/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/i_ga.h"

#include "test/fixture5.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("I_GA")
{

TEST_CASE_FIXTURE(fixture5, "Random creation")
{
  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_ga ind(prob);
    vita::interpreter<vita::i_ga> check(&ind);

    CHECK(ind.debug());
    CHECK(ind.parameters() == prob.sset.categories());
    CHECK(ind.age() == 0);
    CHECK(check.penalty() == doctest::Approx(0.0));
  }
}

TEST_CASE_FIXTURE(fixture5, "Empty individual")
{
  vita::i_ga ind;

  CHECK(ind.debug());
  CHECK(ind.empty());
}


TEST_CASE_FIXTURE(fixture5, "Penalty")
{
  for (unsigned i(0); i < 100; ++i)
  {
    vita::i_ga ind(prob);
    vita::interpreter<vita::i_ga> check(&ind);

    CHECK(check.penalty() == doctest::Approx(0.0));

    ind[0].par = 20.0;
    const auto p1(check.penalty());
    CHECK(p1 > 0.0);

    ind[0].par = -20.0;
    const auto p1n(check.penalty());
    CHECK(p1 == doctest::Approx(p1n));

    ind[1].par = 200.0;
    const auto p2(check.penalty());
    CHECK(p2 > p1);

    ind[1].par = -200.0;
    const auto p2n(check.penalty());
    CHECK(p2 > p1);
    CHECK(p2 == doctest::Approx(p2n));

    ind[2].par = 2000.0;
    const auto p3(check.penalty());
    CHECK(p3 > p2);

    ind[2].par = -2000.0;
    const auto p3n(check.penalty());
    CHECK(p3n > p2);
    CHECK(p3 == doctest::Approx(p3n));
  }
}

TEST_CASE_FIXTURE(fixture5, "Mutation")
{
  vita::i_ga t(prob);
  const vita::i_ga orig(t);

  const unsigned n(1000);

  // Zero probability mutation.
  for (unsigned i(0); i < n; ++i)
  {
    t.mutation(0.0, prob);
    CHECK(t == orig);
  }

  // 50% probability mutation.
  unsigned diff(0);

  for (unsigned i(0); i < n; ++i)
  {
    auto i1(orig);

    i1.mutation(0.5, prob);
    diff += orig.distance(i1);
  }

  const double perc(100.0 * double(diff) / double(orig.parameters() * n));
  CHECK(perc > 47.0);
  CHECK(perc < 53.0);
}

TEST_CASE_FIXTURE(fixture5, "Comparison")
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_ga a(prob);
    CHECK(a == a);
    CHECK(a.distance(a) == 0);

    vita::i_ga b(a);
    CHECK(a.signature() == b.signature());
    CHECK(a == b);
    CHECK(a.distance(b) == 0);

    vita::i_ga c(prob);
    if (a.signature() != c.signature())
    {
      CHECK(!(a == c));
      CHECK(a.distance(c) > 0);
      CHECK(a.distance(c) == c.distance(a));
    }
  }
}

TEST_CASE_FIXTURE(fixture5, "Iterators")
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::i_ga ind(prob);

    unsigned i(0);
    for (const auto &g : ind)
    {
      CHECK(g == ind[i]);
      ++i;
    }
  }
}

TEST_CASE_FIXTURE(fixture5, "Standard crossover")
{
  vita::i_ga i1(prob), i2(prob);

  double dist(0.0);
  const unsigned n(1000);
  for (unsigned j(0); j < n; ++j)
  {
    if (vita::random::boolean())
      i1.inc_age();
    if (vita::random::boolean())
      i2.inc_age();

    const auto ic(crossover(i1, i2));
    CHECK(ic.debug());
    CHECK(ic.age() == std::max(i1.age(), i2.age()));

    dist += i1.distance(ic);
  }

  // +1 since we have at least one gene involved in crossover.
  const double perc(100.0 * dist / ((prob.sset.categories() + 1) * n));
  CHECK(perc > 48.0);
  CHECK(perc < 52.0);
}

TEST_CASE_FIXTURE(fixture5, "Serialization")
{
  // Non-empty i_ga serialization.
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_ga i1(prob);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    CHECK(i1.save(ss));

    vita::i_ga i2(prob);
    CHECK(i2.load(ss, prob));
    CHECK(i2.debug());

    CHECK(i1 == i2);
  }

  // Non-empty i_ga serialization.
  std::stringstream ss;
  vita::i_ga empty;
  CHECK(empty.save(ss));

  vita::i_ga empty1;
  CHECK(empty1.load(ss, prob));
  CHECK(empty1.debug());
  CHECK(empty1.empty());

  CHECK(empty == empty1);

}

}  // TEST_SUITE("I_GA")
