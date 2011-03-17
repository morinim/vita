/**
 *
 *  \file test3.cc
 *
 *  \author Manlio Morini
 *  \date 2011/03/15
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>

#include "boost/assign.hpp"

#include "environment.h"
#include "primitive/sr_pri.h"
#include "evolution.h"

#define BOOST_TEST_MODULE Population
#include "boost/test/included/unit_test.hpp"

using namespace boost;

struct F
{
  F() 
    : num(new vita::sr::number(-200,200)),
      f_add(new vita::sr::add()),
      f_sub(new vita::sr::sub()),
      f_mul(new vita::sr::mul()),
      f_ifl(new vita::sr::ifl()),
      f_ife(new vita::sr::ife())
  { 
    BOOST_TEST_MESSAGE("Setup fixture");
    env.insert(num);
    env.insert(f_add);
    env.insert(f_sub);
    env.insert(f_mul);
    env.insert(f_ifl);
    env.insert(f_ife);
  }

  ~F()
  { 
    BOOST_TEST_MESSAGE("Teardown fixture");
    delete num;
    delete f_add;
    delete f_sub;
    delete f_mul;
    delete f_ifl;
    delete f_ife;
  }

  vita::sr::number *const num;
  vita::sr::add *const f_add;
  vita::sr::sub *const f_sub;
  vita::sr::mul *const f_mul;
  vita::sr::ifl *const f_ifl;
  vita::sr::ife *const f_ife;
  
  vita::environment env;
};


BOOST_FIXTURE_TEST_SUITE(Population,F)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  for (unsigned n(4); n <= 100; ++n)
    for (unsigned l(1); l <= 100; l+=(l < 10 ? 1 : 30))
    {
      env.individuals = n;
      env.code_length = l;

      std::auto_ptr<vita::evaluator> eva(new vita::random_evaluator());
      vita::population p(env);
      vita::evolution evo(env,p,eva.get());

      /*
      if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
      {
        vita::analyzer ay;
        evo.pick_stats(&ay);

        const boost::uint64_t nef(ay.functions(true));
        const boost::uint64_t net(ay.terminals(true));
        const boost::uint64_t ne(nef+net);

        std::cout << std::string(40,'-') << std::endl;
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
