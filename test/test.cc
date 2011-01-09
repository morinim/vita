/**
 *
 *  \file test.cc
 *
 *  \author Manlio Morini
 *  \date 2010/06/10
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

#define BOOST_TEST_MODULE VITA
#include "boost/test/included/unit_test.hpp"

using namespace boost;



template<class T>
T
dummy_fitness(const vita::individual &)
{
  return T(0);
}


// **********************************************************************
BOOST_AUTO_TEST_SUITE(Primitive)

BOOST_AUTO_TEST_CASE(SymbolicRegression)
{
  vita::sr::constant *const c0 = new vita::sr::constant(0);
  vita::sr::constant *const c1 = new vita::sr::constant(1);
  vita::sr::constant *const c2 = new vita::sr::constant(2);
  vita::sr::constant *const c3 = new vita::sr::constant(3);
  vita::sr::constant *const x = new vita::sr::constant(123);
  vita::sr::constant *const neg_x = new vita::sr::constant(-123);
  vita::sr::constant *const y = new vita::sr::constant(321);

  vita::sr::abs  *const f_abs  =  new vita::sr::abs();
  vita::sr::add  *const f_add  =  new vita::sr::add();
  vita::sr::div  *const f_div  =  new vita::sr::div();
  vita::sr::idiv *const f_idiv = new vita::sr::idiv();
  vita::sr::sub  *const f_sub  =  new vita::sr::sub();
  
  vita::environment env;
  env.insert(c0);
  env.insert(c1);
  env.insert(c2);
  env.insert(c3);
  env.insert(x);
  env.insert(neg_x);
  env.insert(y);
  env.insert(f_abs);  
  env.insert(f_add);
  env.insert(f_div);
  env.insert(f_idiv);
  env.code_length = 32;

  vita::individual i(env,true);
  std::vector<unsigned> empty;
  any ret;

  // -- ABS ---------------------------------------------------
  BOOST_TEST_MESSAGE("ABS");
  // *** ABS(-X) = X ***
  i = i.replace(f_abs,assign::list_of(1),0);  // [0] ABS 1
  i = i.replace(neg_x,             empty,1);  // [1] -X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),x->val);
  // *** ABS(X) = X ***
  i = i.replace(f_abs,assign::list_of(1),0);  // [0] ABS 1
  i = i.replace(    x,             empty,1);  // [1] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),x->val);
  
  // -- ADD ---------------------------------------------------
  BOOST_TEST_MESSAGE("ADD");
  // *** X+0 = X ***
  i = i.replace(f_add,assign::list_of(1)(2),0);  // [0] ADD 1,2
  i = i.replace(   c0,                empty,1);  // [1] 0
  i = i.replace(    x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),x->val);
  // *** X+Y is right ***
  i = i.replace(f_add,assign::list_of(1)(2),0);  // [0] ADD 1,2
  i = i.replace(    y,                empty,1);  // [1] Y
  i = i.replace(    x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),y->val+x->val);
  // *** X+(-X) = 0 ***
  i = i.replace(f_add,assign::list_of(1)(2),0);  // [0] ADD 1,2
  i = i.replace(    x,                empty,1);  // [1] X
  i = i.replace(neg_x,                empty,2);  // [2] -X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),0);
  // *** X+Y = Y+X ***
  i = i.replace(f_sub,assign::list_of(1)(2),0);  // [0] ADD 1,2
  i = i.replace(f_add,assign::list_of(3)(4),1);  // [1] ADD 3,4
  i = i.replace(f_add,assign::list_of(4)(3),2);  // [2] ADD 4,3
  i = i.replace(    x,                empty,3);  // [3] X
  i = i.replace(    y,                empty,4);  // [4] Y
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),0);

  // -- DIV ---------------------------------------------------
  BOOST_TEST_MESSAGE("DIV");
  // *** X/X = 1 ***
  i = i.replace(f_div,assign::list_of(1)(2),0);  // [0] DIV 1, 2
  i = i.replace(    x,                empty,1);  // [1] X
  i = i.replace(    x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),1);
  // *** X/1 = X ***
  i = i.replace(f_div,assign::list_of(1)(2),0);  // [0] DIV 1, 2
  i = i.replace(    x,                empty,1);  // [1] X
  i = i.replace(   c1,                empty,2);  // [2] 1
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),x->val);
  // *** -X/X = -1 ***
  i = i.replace(f_div,assign::list_of(1)(2),0);  // [0] DIV 1, 2
  i = i.replace(neg_x,                empty,1);  // [1] -X
  i = i.replace(    x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),-1);
  // -- IDIV ---------------------------------------------------
  BOOST_TEST_MESSAGE("IDIV");
  // *** X/X = 1 ***
  i = i.replace(f_idiv,assign::list_of(1)(2),0);  // [0] DIV 1, 2
  i = i.replace(     x,                empty,1);  // [1] X
  i = i.replace(     x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),1);
  // *** X/1 = X ***
  i = i.replace(f_idiv,assign::list_of(1)(2),0);  // [0] DIV 1, 2
  i = i.replace(     x,                empty,1);  // [1] X
  i = i.replace(    c1,                empty,2);  // [2] 1
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),x->val);
  // *** -X/X = -1 ***
  i = i.replace(f_idiv,assign::list_of(1)(2),0);  // [0] IDIV 1, 2
  i = i.replace( neg_x,                empty,1);  // [1] -X
  i = i.replace(     x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),-1);

  // -- SUB ---------------------------------------------------
  BOOST_TEST_MESSAGE("SUB");
  // *** X-X = 0 ***
  i = i.replace(f_sub,assign::list_of(1)(2),0);  // [0] SUB 1, 2
  i = i.replace(    x,                empty,1);  // [1] X
  i = i.replace(    x,                empty,2);  // [2] X
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),0);
  // *** X-0 = X ***
  i = i.replace(f_sub,assign::list_of(1)(2),0);  // [0] SUB 1, 2
  i = i.replace(    x,                empty,1);  // [1] X
  i = i.replace(   c0,                empty,2);  // [2] 0
  if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
  {
    std::cout << std::string(40,'-') << std::endl;
    i.list(std::cout);
    std::cout << std::string(40,'-') << std::endl;
  }
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_EQUAL(any_cast<double>(ret),x->val);

}

BOOST_AUTO_TEST_SUITE_END()


// **********************************************************************
BOOST_AUTO_TEST_SUITE(Individual)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  vita::environment env;
  
  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  for (unsigned l(1); l < 100; ++l)
  {
    BOOST_TEST_MESSAGE("Individual " << l);

    env.code_length = l;
    vita::individual i(env,true);

    if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
    {
      std::cout << std::string(40,'-') << std::endl;
      i.dump(std::cout);
      std::cout << std::endl;
      i.list(std::cout);
      std::cout << std::endl;
      i.tree(std::cout);
      std::cout << std::string(40,'-') << std::endl;
    }

    BOOST_REQUIRE(i.check());
    BOOST_REQUIRE_EQUAL(i.size(),l);
  }
}

BOOST_AUTO_TEST_SUITE_END()


// **********************************************************************
BOOST_AUTO_TEST_SUITE(Population)

BOOST_AUTO_TEST_CASE(RandomCreation)
{
  vita::environment env;

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  for (unsigned n(4); n <= 100; ++n)
    for (unsigned l(1); l <= 100; l+=(l < 10 ? 1 : 30))
    {
      env.individuals = n;
      env.code_length = l;

      BOOST_TEST_MESSAGE("Population " << n << "_" << l);
      std::auto_ptr<vita::evaluator> eva(new vita::random_evaluator());
      vita::evolution evo(env,eva.get());

      if (unit_test::runtime_config::log_level() <= unit_test::log_messages)
      {
        std::cout << evo.population() <<std::endl;

        vita::analyzer ay;
        evo.pick_stats(&ay);

        const boost::uint64_t nef(ay.functions(true));
        const boost::uint64_t net(ay.terminals(true));
        const boost::uint64_t long ne(nef+net);

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

      BOOST_REQUIRE(evo.check());
    }
}

BOOST_AUTO_TEST_SUITE_END()
