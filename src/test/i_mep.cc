/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <sstream>

#include "kernel/i_mep.h"
#include "kernel/interpreter.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE t_i_mep
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#include "factory_fixture3.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_i_mep_factory3, F_FACTORY3)

BOOST_AUTO_TEST_CASE(t_random_creation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation.");
  for (unsigned l(env.sset->categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::i_mep i(env);

    BOOST_TEST(i.debug());
    BOOST_TEST(i.size() == l);
    BOOST_TEST(i.age() == 0);
  }
}

BOOST_AUTO_TEST_CASE(t_empty_individual)
{
  vita::i_mep i;

  BOOST_TEST(i.debug());
  BOOST_TEST(i.empty());
  BOOST_TEST(i.size() == 0);

  i = vita::i_mep(env);
  BOOST_TEST(!i.empty());
}

BOOST_AUTO_TEST_CASE(t_mutation)
{
  env.code_length = 100;

  vita::i_mep ind(env);
  const vita::i_mep orig(ind);

  const unsigned n(4000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation.");
  for (unsigned i(0); i < n; ++i)
  {
    ind.mutation(0.0, env);
    BOOST_TEST(ind == orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  unsigned diff(0), length(0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::i_mep i1(ind);

    ind.mutation(0.5, env);
    diff += distance(i1, ind);
    length += i1.active_symbols();
  }

  const double perc(100.0 * diff / length);
  BOOST_TEST(perc > 47.0);
  BOOST_TEST(perc < 52.0);
}

BOOST_AUTO_TEST_CASE(t_comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_mep a(env);
    BOOST_TEST(a == a);
    BOOST_TEST(distance(a, a) == 0);

    vita::i_mep b(a);
    BOOST_TEST(a.signature() == b.signature());
    BOOST_TEST(a == b);
    BOOST_TEST(distance(a, b) == 0);

    vita::i_mep c(env);
    if (a.signature() != c.signature())
    {
      BOOST_TEST(a != c);
      BOOST_TEST(distance(a, c) > 0);
      BOOST_TEST(distance(a, c) == distance(c, a));
    }
  }
}

BOOST_AUTO_TEST_CASE(t_crossover)
{
  using namespace vita;

  env.code_length = 100;

  i_mep i1(env), i2(env);

  const unsigned n(2000);
  for (unsigned j(0); j < n; ++j)
  {
    if (random::boolean())
      i1.inc_age();
    if (random::boolean())
      i2.inc_age();

    const auto ic(crossover(i1, i2));
    BOOST_TEST(ic.debug());
    BOOST_TEST(ic.age() == std::max(i1.age(), i2.age()));

    for (index_t i(0); i != ic.size(); ++i)
      for (category_t c(0); c < ic.categories(); ++c)
      {
        const locus l{i, c};

        BOOST_TEST((ic[l] == i1[l] || ic[l] == i2[l]));
      }
  }
}

BOOST_AUTO_TEST_CASE(t_serialization)
{
  BOOST_TEST_CHECKPOINT("Non-empty i_mep serialization");
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_mep i1(env);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    BOOST_TEST(i1.save(ss));

    vita::i_mep i2(env);
    BOOST_TEST(i2.load(ss, env));
    BOOST_TEST(i2.debug());

    BOOST_TEST(i1 == i2);
  }

  BOOST_TEST_CHECKPOINT("Empty i_mep serialization");
  std::stringstream ss;
  vita::i_mep empty;
  BOOST_TEST(empty.save(ss));

  vita::i_mep empty1;
  BOOST_TEST(empty1.load(ss, env));
  BOOST_TEST(empty1.debug());
  BOOST_TEST(empty1.empty());

  BOOST_TEST(empty == empty1);
}

BOOST_AUTO_TEST_CASE(t_blocks)
{
  const unsigned n(1000);

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual with an effective size
    // greater than 4.
    vita::i_mep base(env);
    auto base_es(base.active_symbols());
    while (base_es < 5)
    {
      base = vita::i_mep(env);
      base_es = base.active_symbols();
    }

    auto blk_idx(base.blocks());

    BOOST_TEST(blk_idx.size() > 0);

    for (const auto &l : blk_idx)
    {
      auto blk(base.get_block(l));

      BOOST_TEST(blk.active_symbols() > 1);
      BOOST_TEST(blk[l].sym->arity() > 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(t_output)
{
  vita::i_mep i({
                  {{f_sub, {1, 2}}},  // [0] SUB 1,2
                  {{f_add, {3, 4}}},  // [1] ADD 3,4
                  {{f_add, {4, 3}}},  // [2] ADD 4,3
                  {{   c2,   null}},  // [3] 2.0
                  {{   c3,   null}}   // [4] 3.0
                });

  std::stringstream ss;

  BOOST_TEST_CHECKPOINT("Inline output");
  ss << vita::out::in_line << i;
  BOOST_TEST(ss.str() == "FSUB FADD 2.0 3.0 FADD 3.0 2.0");

  BOOST_TEST_CHECKPOINT("Graphviz output");
  // Typically to 'reset' a stringstream you need to both reset the underlying
  // sequence to an empty string with str and to clear any fail and eof flags
  // with clear.
  ss.clear();
  ss.str(std::string());
  ss << vita::out::graphviz << i;

  BOOST_TEST(ss.str() ==
             "graph {"
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

  BOOST_TEST_CHECKPOINT("Dump output");
  ss.clear();
  ss.str(std::string());
  ss << vita::out::dump << i;

  BOOST_TEST(ss.str() ==
             "[0] FSUB [1] [2]\n"
             "[1] FADD [3] [4]\n"
             "[2] FADD [4] [3]\n"
             "[3] 2.0\n"
             "[4] 3.0\n");

  BOOST_TEST_CHECKPOINT("List output");
  ss.clear();
  ss.str(std::string());
  ss << vita::out::list << i;

  BOOST_TEST(ss.str() ==
             "[0] FSUB [1] [2]\n"
             "[1] FADD 2.0 3.0\n"
             "[2] FADD 3.0 2.0\n");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(t_i_mep_factory1, F_FACTORY1)
BOOST_AUTO_TEST_CASE(t_compress)
{
  const unsigned n(5000);

  for (unsigned k(0); k < n; ++k)
  {
    const vita::i_mep i(env), i1(i.compress());

    BOOST_TEST(i1.debug());

    /*
    std::cout << "\n\n";
    i.list(std::cout, false);
    std::cout << "\n\n";
    i1.list(std::cout, false);
    */

    const auto v(vita::interpreter<vita::i_mep>(&i).run());
    const auto v1(vita::interpreter<vita::i_mep>(&i1).run());

    BOOST_TEST(v.has_value() == v1.has_value());

    if (v.has_value())
    {
      const auto d(vita::to<double>(v));
      const auto d1(vita::to<double>(v1));

      BOOST_TEST(d == d1);
    }

    BOOST_TEST(i1.active_symbols() <= i.active_symbols());

    BOOST_TEST(i.signature() == i1.signature());
  }
}
BOOST_AUTO_TEST_SUITE_END()
