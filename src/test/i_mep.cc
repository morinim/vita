/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/mep/interpreter.h"

#include "fixture1.h"
#include "fixture3.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("I_MEP")
{

TEST_CASE_FIXTURE(fixture3, "Random creation")
{
  // Variable length random creation.
  for (auto l(prob.sset.categories() + 2); l < 100; ++l)
  {
    prob.env.mep.code_length = l;
    vita::i_mep i(prob);

    CHECK(i.is_valid());
    CHECK(i.size() == l);
    CHECK(i.age() == 0);
  }
}

TEST_CASE_FIXTURE(fixture3, "Empty individual")
{
  vita::i_mep i;

  CHECK(i.is_valid());
  CHECK(i.empty());
  CHECK(i.size() == 0);

  i = vita::i_mep(prob);
  CHECK(!i.empty());
}

TEST_CASE_FIXTURE(fixture3, "Mutation")
{
  prob.env.mep.code_length = 100;

  vita::i_mep ind(prob);
  const vita::i_mep orig(ind);

  const unsigned n(4000);

  // Zero probability mutation.
  for (unsigned i(0); i < n; ++i)
  {
    ind.mutation(0.0, prob);
    CHECK(ind == orig);
  }

  // 50% probability mutation.
  unsigned diff(0), length(0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::i_mep i1(ind);

    ind.mutation(0.5, prob);
    diff += distance(i1, ind);
    length += i1.active_symbols();
  }

  const double perc(100.0 * diff / length);
  CHECK(perc > 47.0);
  CHECK(perc < 52.0);
}

TEST_CASE_FIXTURE(fixture3, "Comparison")
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_mep a(prob);
    CHECK(a == a);
    CHECK(distance(a, a) == 0);

    vita::i_mep b(a);
    CHECK(a.signature() == b.signature());
    CHECK(a == b);
    CHECK(distance(a, b) == 0);

    vita::i_mep c(prob);
    if (a.signature() != c.signature())
    {
      CHECK(a != c);
      CHECK(distance(a, c) > 0);
      CHECK(distance(a, c) == distance(c, a));
    }
  }
}

TEST_CASE_FIXTURE(fixture3, "Crossover")
{
  using namespace vita;

  prob.env.mep.code_length = 100;

  i_mep i1(prob), i2(prob);

  const unsigned n(2000);
  for (unsigned j(0); j < n; ++j)
  {
    if (random::boolean())
      i1.inc_age();
    if (random::boolean())
      i2.inc_age();

    const auto ic(crossover(i1, i2));
    CHECK(ic.is_valid());
    CHECK(ic.age() == std::max(i1.age(), i2.age()));

    for (index_t i(0); i != ic.size(); ++i)
      for (category_t c(0); c < ic.categories(); ++c)
      {
        const locus l{i, c};

        CHECK((ic[l] == i1[l] || ic[l] == i2[l]));
      }
  }
}

TEST_CASE_FIXTURE(fixture3, "Serialization")
{
  // Non-empty i_mep serialization.
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_mep i1(prob);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    CHECK(i1.save(ss));

    vita::i_mep i2(prob);
    CHECK(i2.load(ss, prob.sset));
    CHECK(i2.is_valid());

    CHECK(i1 == i2);
  }

  // Empty i_mep serialization.
  std::stringstream ss;
  vita::i_mep empty;
  CHECK(empty.save(ss));

  vita::i_mep empty1;
  CHECK(empty1.load(ss, prob.sset));
  CHECK(empty1.is_valid());
  CHECK(empty1.empty());

  CHECK(empty == empty1);
}

TEST_CASE_FIXTURE(fixture3, "Blocks")
{
  const unsigned n(1000);

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual with an effective size
    // greater than 4.
    vita::i_mep base(prob);
    auto base_es(base.active_symbols());
    while (base_es < 5)
    {
      base = vita::i_mep(prob);
      base_es = base.active_symbols();
    }

    auto blk_idx(base.blocks());

    CHECK(blk_idx.size() > 0);

    for (const auto &l : blk_idx)
    {
      auto blk(base.get_block(l));

      CHECK(blk.active_symbols() > 1);
      CHECK(blk[l].sym->arity() > 0);
    }
  }
}

TEST_CASE_FIXTURE(fixture3, "Output")
{
  vita::i_mep i({
                  {{f_sub, {1, 2}}},  // [0] SUB 1,2
                  {{f_add, {3, 4}}},  // [1] ADD 3,4
                  {{f_add, {4, 3}}},  // [2] ADD 4,3
                  {{   c2,   null}},  // [3] 2.0
                  {{   c3,   null}}   // [4] 3.0
                });

  std::stringstream ss;

  // Inline output.
  ss << vita::out::in_line << i;
  CHECK(ss.str() == "FSUB FADD 2.0 3.0 FADD 3.0 2.0");

  // Graphviz output.
  // Typically to 'reset' a stringstream you need to both reset the underlying
  // sequence to an empty string with str and to clear any fail and eof flags
  // with clear.
  ss.clear();
  ss.str(std::string());
  ss << vita::out::graphviz << i;

  CHECK(ss.str() == "graph {"
                    "g0_0 [label=FSUB, shape=box];"
                    "g0_0 -- g1_0;"
                    "g0_0 -- g2_0;"
                    "g1_0 [label=FADD, shape=box];"
                    "g1_0 -- g3_0;"
                    "g1_0 -- g4_0;"
                    "g2_0 [label=FADD, shape=box];"
                    "g2_0 -- g4_0;"
                    "g2_0 -- g3_0;"
                    "g3_0 [label=2.0, shape=circle];"
                    "g4_0 [label=3.0, shape=circle];}");

  // Dump output.
  ss.clear();
  ss.str(std::string());
  ss << vita::out::dump << i;

  CHECK(ss.str() == "[0] FSUB [1] [2]\n"
                    "[1] FADD [3] [4]\n"
                    "[2] FADD [4] [3]\n"
                    "[3] 2.0\n"
                    "[4] 3.0\n");

  // List output.
  ss.clear();
  ss.str(std::string());
  ss << vita::out::list << i;

  CHECK(ss.str() == "[0] FSUB [1] [2]\n"
                    "[1] FADD 2.0 3.0\n"
                    "[2] FADD 3.0 2.0\n");
}

TEST_CASE_FIXTURE(fixture3, "Common subexpression elimination")
{
  using namespace vita;

  const unsigned n(5000);

  for (unsigned k(0); k < n; ++k)
  {
    const i_mep i(prob), i1(i.cse());

    CHECK(i1.is_valid());

    const auto v(run(i));
    const auto v1(run(i1));

    CHECK(has_value(v) == has_value(v1));

    if (has_value(v))
    {
      const auto d(lexical_cast<D_DOUBLE>(v));
      const auto d1(lexical_cast<D_DOUBLE>(v1));

      CHECK(d == doctest::Approx(d1));
    }

    CHECK(i1.active_symbols() <= i.active_symbols());

    CHECK(i.signature() == i1.signature());
  }
}

TEST_CASE_FIXTURE(fixture3, "Random locus")
{
  using namespace vita;

  for (unsigned k(0); k < 100; ++k)
  {
    std::map<locus, unsigned> exons;

    const i_mep prg(prob);
    CHECK(prg.is_valid());

    const unsigned as(prg.active_symbols());

    const unsigned n(10000);
    for (unsigned j(0); j < n; ++j)
      ++exons[random_locus(prg)];

    const double avg(n / as);
    for (const auto &e : exons)
    {
      CHECK(avg * 0.90 <= e.second);
      CHECK(e.second <= 1.10 *avg);
    }
  }
}

}  // TEST_SUITE("I_MEP")
