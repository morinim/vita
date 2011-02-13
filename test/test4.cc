/**
 *
 *  \file test4.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/31
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <sstream>

#include "boost/assign.hpp"

#include "environment.h"
#include "primitive/sr_pri.h"
#include "evolution.h"

#define BOOST_TEST_MODULE TranspositionTable
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
      f_ife(new vita::sr::ife()),
      cache(16)
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
  vita::ttable cache;
};


BOOST_FIXTURE_TEST_SUITE(TranspositionTable,F)

BOOST_AUTO_TEST_CASE(InsertFindCicle)
{
  env.code_length = 64;

  const unsigned n(6000);

  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env,true);

    cache.insert(i1,i);

    vita::fitness_t f1;
    BOOST_REQUIRE(cache.find(i1,&f1));
    BOOST_REQUIRE_EQUAL(f1,i);
  }
}

BOOST_AUTO_TEST_CASE(CollisionDetection)
{
  env.code_length = 64;

  const unsigned n(1000);

  std::vector<vita::individual> vi;
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env,true);
    const boost::any val(vita::interpreter(i1).run());
    vita::fitness_t f(val.empty() ? 0 : any_cast<vita::fitness_t>(val));

    cache.insert(i1,f);
    vi.push_back(i1);
  }

  for (unsigned i(0); i < n; ++i)
  {
    vita::fitness_t f1;
    if (cache.find(vi[i],&f1))
    {
      const boost::any val(vita::interpreter(vi[i]).run());
      vita::fitness_t f(val.empty() ? 0	: any_cast<vita::fitness_t>(val));
      
      BOOST_CHECK_EQUAL(f1,f);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
