/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/ga/i_de.h"

#include "test/fixture5.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("I_DE")
{

TEST_CASE_FIXTURE(fixture5, "Random creation")
{
  for (unsigned i(0); i < 1000; ++i)
  {
    vita::i_de ind(prob);

    CHECK(ind.is_valid());
    CHECK(ind.parameters() == prob.sset.categories());
    CHECK(ind.age() == 0);

    for (unsigned j(0); j < ind.parameters(); ++j)
      CHECK(std::fabs(ind[j]) <= std::pow(10.0, j + 1));
  }
}

TEST_CASE_FIXTURE(fixture5, "Empty individual")
{
  vita::i_de ind;

  CHECK(ind.is_valid());
  CHECK(ind.empty());
}

TEST_CASE_FIXTURE(fixture5, "Comparison")
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_de a(prob);
    CHECK(a == a);
    CHECK(distance(a, a) == doctest::Approx(0.0));

    vita::i_de b(a);
    CHECK(a.signature() == b.signature());
    CHECK(a == b);
    CHECK(distance(a, b) == doctest::Approx(0.0));

    vita::i_de c(prob);
    if (a.signature() != c.signature())
    {
      CHECK(!(a == c));
      CHECK(distance(a, c) > 0.0);
      CHECK(distance(a, c) == doctest::Approx(distance(c, a)));
    }
  }
}

TEST_CASE_FIXTURE(fixture5, "Iterators")
{
  for (unsigned j(0); j < 1000; ++j)
  {
    vita::i_de ind(prob);

    unsigned i(0);
    for (const auto &v : ind)
    {
      CHECK(v == doctest::Approx(ind[i]));
      ++i;
    }
  }
}

TEST_CASE_FIXTURE(fixture5, "DE crossover")
{
  double diff(0), length(0);

  for (unsigned j(0); j < 1000; ++j)
  {
    const vita::i_de p(prob);
    vita::i_de a(prob), b(prob), c(prob);

    const auto n_a(vita::random::between<unsigned>(0, 100));
    for (unsigned k(0); k < n_a; ++k)
      a.inc_age();
    const auto n_b(vita::random::between<unsigned>(0, 100));
    for (unsigned k(0); k < n_b; ++k)
      b.inc_age();
    const auto n_c(vita::random::between<unsigned>(0, 100));
    for (unsigned k(0); k < n_c; ++k)
      c.inc_age();

    auto off(p.crossover(prob.env.p_cross, prob.env.de.weight, a, a, p));
    CHECK(off.is_valid());

    for (unsigned i(0); i < p.parameters(); ++i)
      CHECK(off[i] == doctest::Approx(p[i]));

    off = p.crossover(prob.env.p_cross, prob.env.de.weight, a, b, p);
    CHECK(off.is_valid());
    CHECK(off.age() == std::max({p.age(), a.age(), b.age()}));

    for (unsigned i(0); i < p.parameters(); ++i)
    {
      const auto delta(prob.env.de.weight.second * std::abs(a[i] - b[i]));

      CHECK(off[i] > p[i] - delta);
      CHECK(off[i] < p[i] + delta);

      if (!vita::almost_equal(p[i], off[i]))
        ++diff;
    }

    off = p.crossover(prob.env.p_cross, prob.env.de.weight, a, b, c);
    CHECK(off.is_valid());
    CHECK(off.age() == std::max({p.age(), a.age(), b.age(), c.age()}));
    for (unsigned i(0); i < p.parameters(); ++i)
    {
      const auto delta(prob.env.de.weight.second * std::abs(a[i] - b[i]));

      if (!vita::almost_equal(p[i], off[i]))
      {
        CHECK(off[i] > c[i] - delta);
        CHECK(off[i] < c[i] + delta);
      }
    }

    length += p.parameters();
  }

  CHECK(diff / length < prob.env.p_cross + 2.0);
  CHECK(diff / length > prob.env.p_cross - 2.0);
}

TEST_CASE_FIXTURE(fixture5, "Serialization")
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_de i1(prob);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    CHECK(i1.save(ss));

    vita::i_de i2(prob);
    CHECK(i2.load(ss));
    CHECK(i2.is_valid());

    CHECK(i1 == i2);
  }

  std::stringstream ss;
  vita::i_de empty;
  CHECK(empty.save(ss));

  vita::i_de empty1;
  CHECK(empty1.load(ss));
  CHECK(empty1.is_valid());
  CHECK(empty1.empty());

  CHECK(empty == empty1);

}

}  // TEST_SUITE("I_DE")
