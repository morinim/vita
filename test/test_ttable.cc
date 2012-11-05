/**
 *
 *  \file test_ttable.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <sstream>

#include "environment.h"
#include "individual.h"
#include "interpreter.h"
#include "primitive/factory.h"

#define BOOST_TEST_MODULE TranspositionTable
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F() : env(true), cache(16)
  {
    BOOST_TEST_MESSAGE("Setup fixture");

    vita::symbol_factory &factory(vita::symbol_factory::instance());

    env.insert(factory.make("REAL", {}));
    env.insert(factory.make("FADD", {}));
    env.insert(factory.make("FSUB", {}));
    env.insert(factory.make("FMUL", {}));
    env.insert(factory.make("FIFL", {}));
    env.insert(factory.make("FIFE", {}));
  }

  ~F()
  {
    BOOST_TEST_MESSAGE("Teardown fixture");
  }

  vita::environment env;
  vita::ttable cache;
};


BOOST_FIXTURE_TEST_SUITE(TranspositionTable, F)

BOOST_AUTO_TEST_CASE(InsertFindCicle)
{
  env.code_length = 64;

  const unsigned n(6000);

  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, true);
    vita::score_t s(i, 0.0);

    cache.insert(i1, s);

    BOOST_REQUIRE(cache.find(i1, &s));
    BOOST_REQUIRE_EQUAL(s.fitness, i);
  }
}

BOOST_AUTO_TEST_CASE(CollisionDetection)
{
  env.code_length = 64;

  const unsigned n(1000);

  std::vector<vita::individual> vi;
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, true);
    const boost::any val( (vita::interpreter(i1))() );
    vita::fitness_t f(val.empty() ? 0 : any_cast<vita::fitness_t>(val));
    vita::score_t s(f, 0.0);

    cache.insert(i1, s);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::score_t s;
    if (cache.find(vi[i], &s))
    {
      const boost::any val((vita::interpreter(vi[i]))());
      vita::fitness_t f(val.empty() ? 0 : any_cast<vita::fitness_t>(val));

      BOOST_CHECK_EQUAL(s.fitness, f);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
