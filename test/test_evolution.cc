/**
 *
 *  \file test_evolution.cc
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
#include <iostream>

#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/primitive/factory.h"

#define BOOST_TEST_MODULE Evolution
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F() : env(true)
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
};


BOOST_FIXTURE_TEST_SUITE(Evolution, F)

BOOST_AUTO_TEST_CASE(Creation)
{
  for (unsigned n(4); n <= 100; ++n)
    for (unsigned l(env.sset.categories() + 2); l <= 100; l+= (l < 10 ? 1 : 30))
    {
      env.individuals = n;
      env.code_length = l;

      std::auto_ptr<vita::evaluator> eva(new vita::random_evaluator());
      vita::evolution evo(env, eva.get());

      /*
      if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
      {
        vita::analyzer ay;
        evo.pick_stats(&ay);

        const boost::uint64_t nef(ay.functions(true));
        const boost::uint64_t net(ay.terminals(true));
        const boost::uint64_t ne(nef + net);

        std::cout << std::string(40, '-') << std::endl;
        for (vita::analyzer::const_iterator i(ay.begin());
             i != ay.end();
             ++i)
          std::cout << std::setfill(' ') << (i->first)->display() << ": "
                    << std::setw(5) << i->second.counter[true]
                    << " (" << std::setw(3) << 100*i->second.counter[true]/ne
                    << "%)" << std::endl;

        std::cout << "Average code length: " << ay.length_dist().mean
                  << std::endl
                  << "Code length standard deviation: "
                  << std::sqrt(ay.length_dist().variance) << std::endl
                  << "Max code length: " << ay.length_dist().max << std::endl
                  << "Functions: " << nef << " (" << nef*100/ne << "%)"
                  << std::endl
                  << "Terminals: " << net << " (" << net*100/ne << "%)"
                  << std::endl << std::string(40,'-') << std::endl;
      }
      */

      BOOST_REQUIRE(evo.check());
    }
}

BOOST_AUTO_TEST_SUITE_END()
