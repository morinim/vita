/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
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
#include "kernel/timer.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE symbol_set
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture3.h"
#endif

BOOST_FIXTURE_TEST_SUITE(symbol_set, F_FACTORY3)

BOOST_AUTO_TEST_CASE(Speed)
{
  const unsigned n(10000000);

  // Because of s the compiler have to perform the entire for loop (see below).
  vita::symbol *s(sset.roulette());

  vita::timer t;
  for (unsigned i(0); i < n; ++i)
    if (vita::random::boolean())
      s = sset.roulette();

  BOOST_TEST_MESSAGE(static_cast<unsigned>(1000.0 * n / t.elapsed())
                     << " extractions/sec - symbol: " << s->display());
}

BOOST_AUTO_TEST_CASE(Distribution)
{
  std::map<const vita::symbol *, unsigned> hist, weight;

  const unsigned n(20000000);
  for (unsigned i(0); i < n; ++i)
  {
    const vita::symbol *s(sset.roulette());
    ++hist[s];
    weight[s] = sset.weight(s);
  }

  double sum(0.0);
  for (const auto &i : weight)
    sum += i.second;

  for (const auto &i : hist)
  {
    const auto p(static_cast<double>(weight[i.first]) / sum);
    const auto actual(static_cast<double>(i.second) / n);
    BOOST_CHECK_CLOSE(p, actual, 1.0);
  }
}

BOOST_AUTO_TEST_SUITE_END()
