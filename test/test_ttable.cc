/**
 *
 *  \file test_ttable.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdlib>
#include <sstream>

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"
#include "kernel/primitive/factory.h"

#define BOOST_TEST_MODULE TranspositionTable
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F() : cache(16)
  {
    BOOST_TEST_MESSAGE("Setup fixture");

    vita::symbol_factory &factory(vita::symbol_factory::instance());

    env.insert(factory.make("NUMBER", vita::d_double, {}));
    env.insert(factory.make("ADD", vita::d_double, {}));
    env.insert(factory.make("SUB", vita::d_double, {}));
    env.insert(factory.make("MUL", vita::d_double, {}));
    env.insert(factory.make("IFL", vita::d_double, {}));
    env.insert(factory.make("IFE", vita::d_double, {}));
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
    vita::eva_pair p(i, 0.0);

    cache.insert(i1, p);

    BOOST_REQUIRE(cache.find(i1, &p));
    BOOST_REQUIRE_EQUAL(p.first, i);
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
    vita::eva_pair p(f, 0.0);

    cache.insert(i1, p);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::eva_pair p;
    if (cache.find(vi[i], &p))
    {
      const boost::any val((vita::interpreter(vi[i]))());
      vita::fitness_t f(val.empty() ? 0 : any_cast<vita::fitness_t>(val));

      BOOST_CHECK_EQUAL(p.first, f);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
