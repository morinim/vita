/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
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

#include "factory_fixture3.h"
#endif

BOOST_FIXTURE_TEST_SUITE(t_i_mep, F_FACTORY3)
/*
BOOST_AUTO_TEST_CASE(Compact)
{
  env.code_length = 100;

  std::cout << env.sset << std::endl;

  BOOST_TEST_CHECKPOINT("Functional equivalence.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::i_mep i1(env, true);
    const vita::i_mep i2(i1.compact());

    std::cout << i1 << std::endl << i2 << std::endl;
    BOOST_REQUIRE(i1.debug(true));

    const boost::any v1( (vita::interpreter(i1))() );
    const boost::any v2( (vita::interpreter(i2))() );

    BOOST_REQUIRE_EQUAL(v1.empty(), v2.empty());
    if (!v1.empty() && !v2.empty())
      BOOST_REQUIRE_EQUAL(vita::interpreter::to_string(v1),
                          vita::interpreter::to_string(v2));
  }

  BOOST_TEST_CHECKPOINT("Not interleaved active symbols.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::i_mep ind(vita::i_mep(env, true).compact());

    unsigned line(0), old_line(0);
    for (vita::i_mep::const_iterator it(ind); it(); line = ++it)
      if (line)
      {
        BOOST_REQUIRE_EQUAL(old_line, line-1);
        ++old_line;
      }
  }

  BOOST_TEST_CHECKPOINT("Same signature.");
  for (unsigned n(0); n < 1000; ++n)
  {
    const vita::i_mep i1(env, true);
    const vita::i_mep i2(i1.compact());

    BOOST_REQUIRE_EQUAL(i1.signature(), i2.signature());
  }
}
*/

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  BOOST_TEST_CHECKPOINT("Variable length random creation.");
  for (unsigned l(sset.categories() + 2); l < 100; ++l)
  {
    env.code_length = l;
    vita::i_mep i(env, sset);

    BOOST_REQUIRE(i.debug());
    BOOST_REQUIRE_EQUAL(i.size(), l);
    BOOST_REQUIRE_EQUAL(i.age(), 0);
  }
}

BOOST_AUTO_TEST_CASE(EmptyIndividual)
{
  vita::i_mep i;

  BOOST_REQUIRE(i.debug());
}

BOOST_AUTO_TEST_CASE(Mutation)
{
  env.code_length = 100;

  vita::i_mep ind(env, sset);
  const vita::i_mep orig(ind);

  const unsigned n(4000);

  BOOST_TEST_CHECKPOINT("Zero probability mutation.");
  env.p_mutation = 0.0;
  for (unsigned i(0); i < n; ++i)
  {
    ind.mutation();
    BOOST_REQUIRE_EQUAL(ind, orig);
  }

  BOOST_TEST_CHECKPOINT("50% probability mutation.");
  env.p_mutation = 0.5;
  unsigned diff(0), length(0);

  for (unsigned i(0); i < n; ++i)
  {
    const vita::i_mep i1(ind);

    ind.mutation();
    diff += i1.distance(ind);
    length += i1.eff_size();
  }

  const double perc(100.0 * double(diff) / double(length));
  BOOST_CHECK_GT(perc, 47.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(Comparison)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    vita::i_mep a(env, sset);
    BOOST_REQUIRE_EQUAL(a, a);
    BOOST_REQUIRE_EQUAL(a.distance(a), 0);

    vita::i_mep b(a);
    BOOST_REQUIRE_EQUAL(a.signature(), b.signature());
    BOOST_REQUIRE_EQUAL(a, b);
    BOOST_REQUIRE_EQUAL(a.distance(b), 0);

    vita::i_mep c(env, sset);
    if (a.signature() != c.signature())
    {
      BOOST_REQUIRE_NE(a, c);
      BOOST_REQUIRE_GT(a.distance(c), 0);
      BOOST_REQUIRE_EQUAL(a.distance(c), c.distance(a));
    }
  }
}

BOOST_AUTO_TEST_CASE(Crossover)
{
  env.code_length = 100;

  vita::i_mep i1(env, sset), i2(env, sset);

  const unsigned n(2000);
  double dist(0.0);
  for (unsigned j(0); j < n; ++j)
  {
    if (vita::random::boolean())
      i1.inc_age();
    if (vita::random::boolean())
      i2.inc_age();

    const auto ic(i1.crossover(i2));
    BOOST_CHECK(ic.debug(true));
    BOOST_REQUIRE_EQUAL(ic.age(), std::max(i1.age(), i2.age()));

    dist += i1.distance(ic);
  }

  const double perc(100.0 * dist / (env.code_length * sset.categories() * n));
  BOOST_CHECK_GT(perc, 45.0);
  BOOST_CHECK_LT(perc, 52.0);
}

BOOST_AUTO_TEST_CASE(Serialization)
{
  for (unsigned i(0); i < 2000; ++i)
  {
    std::stringstream ss;
    vita::i_mep i1(env, sset);

    for (auto j(vita::random::between(0u, 100u)); j; --j)
      i1.inc_age();

    BOOST_REQUIRE(i1.save(ss));

    vita::i_mep i2(env, sset);
    BOOST_REQUIRE(i2.load(ss));
    BOOST_REQUIRE(i2.debug());

    BOOST_CHECK_EQUAL(i1, i2);
  }
}

BOOST_AUTO_TEST_CASE(Blocks)
{
  const unsigned n(1000);

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual with an effective size
    // greater than 4.
    vita::i_mep base(env, sset);
    auto base_es(base.eff_size());
    while (base_es < 5)
    {
      base = vita::i_mep(env, sset);
      base_es = base.eff_size();
    }

    auto blk_idx(base.blocks());

    BOOST_REQUIRE_GT(blk_idx.size(), 0);

    for (const auto &l : blk_idx)
    {
      auto blk(base.get_block(l));

      BOOST_REQUIRE_GT(blk.eff_size(), 1);
      BOOST_REQUIRE_GT(blk[l].sym->arity(), 0);
    }
  }
}

BOOST_AUTO_TEST_CASE(Output)
{
  vita::i_mep i(env, sset,
                {
                  {{f_sub, {1, 2}}},  // [0] SUB 1,2
                  {{f_add, {3, 4}}},  // [1] ADD 3,4
                  {{f_add, {4, 3}}},  // [2] ADD 4,3
                  {{   c2,   null}},  // [3] 2.0
                  {{   c3,   null}}   // [4] 3.0
                });

  std::stringstream ss;

  BOOST_TEST_CHECKPOINT("Inline output");
  i.in_line(ss);
  BOOST_CHECK_EQUAL(ss.str(), "FSUB FADD 2.0 3.0 FADD 3.0 2.0");

  BOOST_TEST_CHECKPOINT("Graphviz output");
  // Typically to 'reset' a stringstream you need to both reset the underlying
  // sequence to an empty string with str and to clear any fail and eof flags
  // with clear.
  ss.clear();
  ss.str(std::string());

  i.graphviz(ss);
  BOOST_CHECK_EQUAL(ss.str(),
                    "graph {" \
                    "g0_0 [label=FSUB, shape=box];" \
                    "g0_0 -- g1_0;" \
                    "g0_0 -- g2_0;" \
                    "g1_0 [label=FADD, shape=box];" \
                    "g1_0 -- g3_0;" \
                    "g1_0 -- g4_0;" \
                    "g2_0 [label=FADD, shape=box];" \
                    "g2_0 -- g4_0;" \
                    "g2_0 -- g3_0;" \
                    "g3_0 [label=2.0, shape=circle];" \
                    "g4_0 [label=3.0, shape=circle];}");
}
BOOST_AUTO_TEST_SUITE_END()
