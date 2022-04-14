/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>
#include <map>

#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/src/primitive/factory.h"
#include "kernel/random.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("SYMBOL_SET")
{

TEST_CASE("Constructor / Insertion")
{
  vita::problem prob;

  // Empty symbol set
  CHECK(prob.sset.categories() == 0);
  CHECK(prob.sset.enough_terminals());
  CHECK(prob.sset.is_valid());

  vita::symbol_factory factory;

  // Undersized symbol set
  auto *fadd = prob.sset.insert(factory.make("FADD", {0}));

  CHECK(prob.sset.categories() == 1);
  CHECK(prob.sset.terminals(0) == 0);
  CHECK(!prob.sset.enough_terminals());

  auto *fsub = prob.sset.insert(factory.make("FSUB", {0}));

  CHECK(prob.sset.categories() == 1);
  CHECK(prob.sset.terminals(0) == 0);
  CHECK(!prob.sset.enough_terminals());
  CHECK(prob.sset.weight(*fadd) == prob.sset.weight(*fsub));

  // Single category symbol set
  auto *real = prob.sset.insert(factory.make("REAL", {0}));

  CHECK(prob.sset.categories() == 1);
  CHECK(prob.sset.terminals(0) == 1);
  CHECK(prob.sset.enough_terminals());
  CHECK(prob.sset.weight(*fadd) == prob.sset.weight(*real));
  CHECK(prob.sset.is_valid());

  auto *sife = prob.sset.insert(factory.make("SIFE", {1, 0}));

  CHECK(prob.sset.categories() == 1);
  CHECK(prob.sset.terminals(0) == 1);
  CHECK(!prob.sset.enough_terminals());
  CHECK(prob.sset.weight(*fadd) == prob.sset.weight(*sife));

  // Multi category symbol set
  auto *apple = prob.sset.insert(factory.make("apple", {1}));

  CHECK(prob.sset.categories() == 2);
  CHECK(prob.sset.terminals(0) == 1);
  CHECK(prob.sset.terminals(1) == 1);
  CHECK(prob.sset.enough_terminals());
  CHECK(prob.sset.weight(*fadd) == prob.sset.weight(*apple));

  CHECK(prob.sset.is_valid());

  CHECK(prob.sset.decode("\"apple\"") == apple);
  CHECK(prob.sset.decode(apple->opcode()) == apple);
  CHECK(prob.sset.decode("SIFE") == sife);
  CHECK(prob.sset.decode(sife->opcode()) == sife);
  CHECK(prob.sset.decode("FSUB") == fsub);
  CHECK(prob.sset.decode(fsub->opcode()) == fsub);
  CHECK(prob.sset.decode("FADD") == fadd);
  CHECK(prob.sset.decode(fadd->opcode()) == fadd);

  // Reset
  prob.sset.clear();

  CHECK(prob.sset.categories() == 0);
  CHECK(prob.sset.enough_terminals());
  CHECK(prob.sset.is_valid());
}

TEST_CASE("Distribution")
{
  vita::problem prob;
  vita::symbol_factory factory;

  // Initial setup
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
      CHECK(prob.sset.weight(*s) / base == doctest::Approx(wanted.at(s)));
  }

  const unsigned n(600000);

  // Roulette_function
  std::map<const vita::symbol *, double> hist;
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette_function(0)];

  for (const auto *s : symbols[0])
    if (!s->terminal())
    {
      CHECK(hist[s] > 0.0);
      CHECK(ratio(hist, s) == doctest::Approx(ratio(wanted, s)).epsilon(0.02));
    }

  // Roulette_terminal
  hist.clear();
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette_terminal(vita::random::boolean())];

  for (vita::category_t c(0); c < prob.sset.categories(); ++c)
    for (const auto *s : symbols[c])
      if (s->terminal())
      {
        CHECK(hist[s] > 0.0);
        CHECK(ratio(hist, s)
              == doctest::Approx(ratio(wanted, s)).epsilon(0.02));
      }

  // Roulette
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
  CHECK(sum_f == doctest::Approx(sum_t).epsilon(0.03));

  for (const auto *s : symbols[0])
  {
    CHECK(hist[s] > 0.0);

    if (s->terminal())
      CHECK(doctest::Approx(ratio(hist, s)).epsilon(0.02)
            == ratio(wanted, s) * sum_f / sum_t);
  }

  // Roulette_free
  hist.clear();
  for (unsigned i(0); i < n; ++i)
    ++hist[&prob.sset.roulette_free(vita::random::boolean())];

  for (vita::category_t c(0); c < prob.sset.categories(); ++c)
    for (const auto *s : symbols[c])
    {
      CHECK(hist[s] > 0.0);
      CHECK(ratio(hist, s) == doctest::Approx(ratio(wanted, s)).epsilon(0.02));
    }
}

}  // TEST_SUITE("SYMBOL_SET")
