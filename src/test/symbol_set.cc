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

#include <cstdlib>
#include <iostream>
#include <map>

#include "kernel/i_mep.h"
#include "kernel/random.h"
#include "utility/timer.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE symbol_set
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture1.h"
#endif

BOOST_AUTO_TEST_SUITE(symbol_set)

/*
BOOST_AUTO_TEST_CASE(Speed)
{
  const unsigned n(10000000);

  // Because of s the compiler have to perform the entire for loop (see below).
  const vita::symbol *s(&prob.sset.roulette(0));

  vita::timer t;
  for (unsigned i(0); i < n; ++i)
    s = &prob.sset.roulette(0);

  BOOST_TEST_MESSAGE(1000.0 * n / t.elapsed().count()
                     << " extractions/sec - symbol: " << s->name());
}
*/

BOOST_AUTO_TEST_CASE(Constructor_Insertion)
{
  vita::problem prob;

  BOOST_TEST_CHECKPOINT("Empty symbol set");
  BOOST_TEST(prob.sset.categories() == 0);
  BOOST_TEST(prob.sset.enough_terminals());
  BOOST_TEST(prob.sset.arg(1).name() == "ARG_1");
  BOOST_TEST(prob.sset.debug());
  BOOST_TEST(prob.sset.adts().empty());

  vita::symbol_factory factory;

  BOOST_TEST_CHECKPOINT("Undersized symbol set");
  auto *fadd = prob.sset.insert(factory.make("FADD", {0}));

  BOOST_TEST(prob.sset.categories() == 1);
  BOOST_TEST(prob.sset.terminals(0) == 0);
  BOOST_TEST(!prob.sset.enough_terminals());

  auto *fsub = prob.sset.insert(factory.make("FSUB", {0}));

  BOOST_TEST(prob.sset.categories() == 1);
  BOOST_TEST(prob.sset.terminals(0) == 0);
  BOOST_TEST(!prob.sset.enough_terminals());
  BOOST_TEST(prob.sset.weight(*fadd) == prob.sset.weight(*fsub));

  BOOST_TEST_CHECKPOINT("Single category symbol set");
  auto *real = prob.sset.insert(factory.make("REAL", {0}));

  BOOST_TEST(prob.sset.categories() == 1);
  BOOST_TEST(prob.sset.terminals(0) == 1);
  BOOST_TEST(prob.sset.enough_terminals());
  BOOST_TEST(prob.sset.weight(*fadd) == prob.sset.weight(*real));
  BOOST_TEST(prob.sset.debug());

  auto *sife = prob.sset.insert(factory.make("SIFE", {1, 0}));

  BOOST_TEST(prob.sset.categories() == 1);
  BOOST_TEST(prob.sset.terminals(0) == 1);
  BOOST_TEST(!prob.sset.enough_terminals());
  BOOST_TEST(prob.sset.weight(*fadd) == prob.sset.weight(*sife));

  BOOST_TEST_CHECKPOINT("Multi category symbol set");
  auto *apple = prob.sset.insert(factory.make("apple", {1}));

  BOOST_TEST(prob.sset.categories() == 2);
  BOOST_TEST(prob.sset.terminals(0) == 1);
  BOOST_TEST(prob.sset.terminals(1) == 1);
  BOOST_TEST(prob.sset.enough_terminals());
  BOOST_TEST(prob.sset.weight(*fadd) == prob.sset.weight(*apple));

  BOOST_TEST(prob.sset.debug());

  BOOST_TEST(prob.sset.decode("\"apple\"") == apple);
  BOOST_TEST(prob.sset.decode(apple->opcode()) == apple);
  BOOST_TEST(prob.sset.decode("SIFE") == sife);
  BOOST_TEST(prob.sset.decode(sife->opcode()) == sife);
  BOOST_TEST(prob.sset.decode("FSUB") == fsub);
  BOOST_TEST(prob.sset.decode(fsub->opcode()) == fsub);
  BOOST_TEST(prob.sset.decode("FADD") == fadd);
  BOOST_TEST(prob.sset.decode(fadd->opcode()) == fadd);

  BOOST_TEST(prob.sset.adts().empty());

  BOOST_TEST_CHECKPOINT("Reset");
  prob.sset.clear();

  BOOST_TEST(prob.sset.categories() == 0);
  BOOST_TEST(prob.sset.enough_terminals());
  BOOST_TEST(prob.sset.debug());
}

BOOST_AUTO_TEST_CASE(Distribution, * boost::unit_test::tolerance(0.02))
{
  vita::problem prob;
  vita::symbol_factory factory;

  BOOST_TEST_CHECKPOINT("Initial setup");
  const std::vector<const vita::symbol *> symbols[2] =
  {
    {
      prob.sset.insert(factory.make("REAL", {0}), 4.0),
      prob.sset.insert(factory.make("FADD", {0}), 3.0),
      prob.sset.insert(factory.make("FSUB", {0}), 2.0),
      prob.sset.insert(factory.make("SIFE", {1, 0}), 2.0),
      prob.sset.insert(factory.make("FMUL", {0}), 1.0)
    },
    {
      prob.sset.insert(factory.make("apple", {1}), 3.0),
      prob.sset.insert(factory.make("orange", {1}), 1.0)
    }
  };

  const std::map<const vita::symbol *, double> wanted =
  {
    {symbols[0][0], 4.0},
    {symbols[0][1], 3.0},
    {symbols[0][2], 2.0},
    {symbols[0][3], 2.0},
    {symbols[0][4], 1.0},
    {symbols[1][0], 3.0},
    {symbols[1][1], 1.0}
  };

  const auto ratio = [&symbols](const auto &container, const vita::symbol *sym)
  {
    const auto val(container.at(sym));

    const vita::symbol *ref(sym->terminal() ? symbols[sym->category()][0]
                                            : symbols[sym->category()].back());
    assert(ref->category() == sym->category());
    assert(ref->terminal() == sym->terminal());

    const auto ref_val(container.at(ref));
    assert(ref_val > 0);

    return val / ref_val;
  };

  for (vita::category_t c(0); c < prob.sset.categories(); ++c)
  {
    const auto base(static_cast<double>(prob.sset.weight(*symbols[c].back())));

    for (const auto *s : symbols[c])
      BOOST_TEST(prob.sset.weight(*s) / base == wanted.at(s));
  }

  const unsigned n(600000);

  BOOST_TEST_CHECKPOINT("roulette_function");
  std::map<const vita::symbol *, double> hist;
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette_function(0)];

  for (const auto *s : symbols[0])
    if (!s->terminal())
    {
      BOOST_TEST(hist[s] > 0.0);
      BOOST_TEST(ratio(hist, s) == ratio(wanted, s));
    }

  BOOST_TEST_CHECKPOINT("roulette_terminal");
  hist.clear();
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette_terminal(vita::random::boolean())];

  for (vita::category_t c(0); c < prob.sset.categories(); ++c)
    for (const auto *s : symbols[c])
      if (s->terminal())
      {
        BOOST_TEST(hist[s] > 0.0);
        BOOST_TEST(ratio(hist, s) == ratio(wanted, s));
      }

  BOOST_TEST_CHECKPOINT("roulette");
  hist.clear();
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette(0)];

  const double sum_f(std::accumulate(
                       hist.begin(), hist.end(), 0.0,
                       [](auto sum, auto e)
                       {
                         return e.first->terminal() ? sum : sum + e.second;
                       }
                    ));
  const double sum_t(std::accumulate(
                       hist.begin(), hist.end(), 0.0,
                       [](auto sum, auto e)
                       {
                         return e.first->terminal() ? sum + e.second : sum;
                       }
                     ));
  BOOST_TEST(sum_f == sum_t, boost::test_tools::tolerance(0.03));

  for (const auto *s : symbols[0])
  {
    BOOST_TEST(hist[s] > 0.0);

    if (s->terminal())
      BOOST_TEST(ratio(hist, s) == ratio(wanted, s) * sum_f / sum_t);
  }

  BOOST_TEST_CHECKPOINT("roulette_free");
  hist.clear();
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette_free(vita::random::boolean())];

  for (vita::category_t c(0); c < prob.sset.categories(); ++c)
    for (const auto *s : symbols[c])
    {
      BOOST_TEST(hist[s] > 0.0);
      BOOST_TEST(ratio(hist, s) == ratio(wanted, s));
    }
}

BOOST_AUTO_TEST_SUITE_END()
