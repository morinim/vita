/**
 *
 *  \file test_primitive.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>

#include "environment.h"
#include "individual.h"
#include "interpreter.h"
#include "random.h"
#include "terminal.h"
#include "primitive/factory.h"

#define BOOST_TEST_MODULE Primitive
#include "boost/test/unit_test.hpp"

using namespace boost;
using namespace vita;

struct F
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0, true) {}

    vita::any eval(vita::interpreter *) const { return vita::any(val); }

    double val;
  };

  F() : env(true), l0(locus{{0, 0}})
  {
    BOOST_TEST_MESSAGE("Setup fixture");

    static vita::symbol_factory &factory(vita::symbol_factory::instance());

    c0 = factory.make("0.0", {});
    c1 = factory.make("1.0", {});
    c2 = factory.make("2.0", {});
    c3 = factory.make("3.0", {});
    x = factory.make("123.0", {});
    neg_x = factory.make("-123.0", {});
    y = factory.make("321.0", {});
    z = std::make_shared<Z>();
    f_abs = factory.make("FABS", {});
    f_add = factory.make("FADD", {});
    f_div = factory.make("FDIV", {});
    f_idiv = factory.make("FIDIV", {});
    f_ln = factory.make("FLN", {});
    f_mul = factory.make("FMUL", {});
    f_sub = factory.make("FSUB", {});

    env.insert(c0);
    env.insert(c1);
    env.insert(c2);
    env.insert(c3);
    env.insert(x);
    env.insert(neg_x);
    env.insert(y);
    env.insert(z);
    env.insert(f_abs);
    env.insert(f_add);
    env.insert(f_div);
    env.insert(f_idiv);
    env.insert(f_mul);
    env.insert(f_sub);
    env.code_length = 32;
  }

  ~F()
  {
    BOOST_TEST_MESSAGE("Teardown fixture");
  }

  vita::symbol_ptr c0;
  vita::symbol_ptr c1;
  vita::symbol_ptr c2;
  vita::symbol_ptr c3;
  vita::symbol_ptr x;
  vita::symbol_ptr neg_x;
  vita::symbol_ptr y;
  vita::symbol_ptr z;

  vita::symbol_ptr f_abs;
  vita::symbol_ptr f_add;
  vita::symbol_ptr f_div;
  vita::symbol_ptr f_idiv;
  vita::symbol_ptr f_ln;
  vita::symbol_ptr f_mul;
  vita::symbol_ptr f_sub;

  vita::environment env;
  vita::any ret;

  const vita::locus l0;
};

BOOST_FIXTURE_TEST_SUITE(SymbolicRegressionSymbols, F)

BOOST_AUTO_TEST_CASE(ABS)
{
  vita::individual i(env, true);

  // With a nonconformant C++11 compiler the {1} expression can be changed
  // with boost::assign::list_of(1).
  BOOST_TEST_CHECKPOINT("ABS(-X) == X");
  i = i.replace(f_abs, {1}, l0 + 0);  // [0] ABS 1
  i = i.replace(neg_x,  {}, l0 + 1);  // [1] -X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(interpreter::to_string(ret) ==
                        interpreter::to_string(x->eval(0)), "\n" << i);

  BOOST_TEST_CHECKPOINT("ABS(X) == X");
  i = i.replace(f_abs, {1}, l0 + 0);  // [0] ABS 1
  i = i.replace(    x,  {}, l0 + 1);  // [1] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(x->eval(0)),
                        "\n" << i);
}

BOOST_AUTO_TEST_CASE(ADD)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("ADD(X,0) == X");
  i = i.replace(f_add, {1, 2}, l0 + 0);  // [0] ADD 1,2
  i = i.replace(   c0,     {}, l0 + 1);  // [1] 0
  i = i.replace(    x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y)=X+Y");
  i = i.replace(f_add, {1, 2}, l0 + 0);  // [0] ADD 1,2
  i = i.replace(    y,     {}, l0 + 1);  // [1] Y
  i = i.replace(    x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(y->eval(0)) +
                        vita::any_cast<double>(x->eval(0)), "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,-X) == 0");
  i = i.replace(f_add, {1, 2}, l0 + 0);  // [0] ADD 1,2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(neg_x,     {}, l0 + 2);  // [2] -X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == ADD(Y,X)");
  i = i.replace(f_sub, {1, 2}, l0 + 0);  // [0] SUB 1,2
  i = i.replace(f_add, {3, 4}, l0 + 1);  // [1] ADD 3,4
  i = i.replace(f_add, {4, 3}, l0 + 2);  // [2] ADD 4,3
  i = i.replace(    x,     {}, l0 + 3);  // [3] X
  i = i.replace(    y,     {}, l0 + 4);  // [4] Y
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(DIV)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("DIV(X,X) == 1");
  i = i.replace(f_div, {1, 2}, l0 + 0);  // [0] DIV 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(    x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(X,1) == X");
  i = i.replace(f_div, {1, 2}, l0 + 0);  // [0] DIV 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(   c1,     {}, l0 + 2);  // [2] 1
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(-X,X) == -1");
  i = i.replace(f_div, {1, 2}, l0 + 0);  // [0] DIV 1, 2
  i = i.replace(neg_x,     {}, l0 + 1);  // [1] -X
  i = i.replace(    x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == -1, "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(X,0) == nan");
  i = i.replace(f_div, {1, 2}, l0 + 0);  // [0] DIV 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(   c0,     {}, l0 + 2);  // [2] 0
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i);
}

BOOST_AUTO_TEST_CASE(IDIV)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("IDIV(X,X) == 1");
  i = i.replace(f_idiv, {1, 2}, l0 + 0);  // [0] DIV 1, 2
  i = i.replace(     x,     {}, l0 + 1);  // [1] X
  i = i.replace(     x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(X,1) == X");
  i = i.replace(f_idiv, {1, 2}, l0 + 0);  // [0] DIV 1, 2
  i = i.replace(     x,     {}, l0 + 1);  // [1] X
  i = i.replace(    c1,     {}, l0 + 2);  // [2] 1
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(-X,X) == -1");
  i = i.replace(f_idiv, {1, 2}, l0 + 0);  // [0] IDIV 1, 2
  i = i.replace( neg_x,     {}, l0 + 1);  // [1] -X
  i = i.replace(     x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == -1, "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(3,2) == 1");
  i = i.replace(f_idiv, {1, 2}, l0 + 0);  // [0] IDIV 1, 2
  i = i.replace(    c3,     {}, l0 + 1);  // [1] 3
  i = i.replace(    c2,     {}, l0 + 2);  // [2] 2
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(X,0) == nan");
  i = i.replace(f_idiv, {1, 2}, l0 + 0);  // [0] IDIV 1, 2
  i = i.replace (    x,     {}, l0 + 1);  // [1] X
  i = i.replace(    c0,     {}, l0 + 2);  // [2] 0
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i);
}

BOOST_AUTO_TEST_CASE(MUL)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("MUL(X,0) == 0");
  i = i.replace(f_mul, {1, 2}, l0 + 0);  // [0] MUL 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(   c0,     {}, l0 + 2);  // [2] 0
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("MUL(X,1) == X");
  i = i.replace(f_mul, {1, 2}, l0 + 0);  // [0] MUL 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(   c1,     {}, l0 + 2);  // [2] 1
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("MUL(X,2) == ADD(X,X)");
  i = i.replace(f_sub, {1, 2}, l0 + 0);  // [0] SUB 1, 2
  i = i.replace(f_add, {3, 3}, l0 + 1);  // [1] ADD 3, 3
  i = i.replace(f_mul, {3, 4}, l0 + 2);  // [2] MUL 3, 4
  i = i.replace(    x,     {}, l0 + 3);  // [3] X
  i = i.replace(   c2,     {}, l0 + 4);  // [4] 2
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 0.0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(SUB)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("SUB(X,-X) == 0");
  i = i.replace(f_sub, {1, 2}, l0 + 0);  // [0] SUB 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(    x,     {}, l0 + 2);  // [2] X
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("SUB(X,0) == X");
  i = i.replace(f_sub, {1, 2}, l0 + 0);  // [0] SUB 1, 2
  i = i.replace(    x,     {}, l0 + 1);  // [1] X
  i = i.replace(   c0,     {}, l0 + 2);  // [2] 0
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                        vita::any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("SUB(Z,X) == Z-X");
  i = i.replace(f_sub, {1, 2}, l0 + 0);  // [0] SUB 1, 2
  i = i.replace(    z,     {}, l0 + 1);  // [1] Z
  i = i.replace(    x,     {}, l0 + 2);  // [2] X
  for (unsigned j(0); j < 1000; ++j)
  {
    static_pointer_cast<Z>(z)->val = vita::random::between<double>(-1000, 1000);
    ret = vita::interpreter(i).run();
    BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                          static_pointer_cast<Z>(z)->val -
                          vita::any_cast<double>(x->eval(0)),
      "\n" << i);
  }
}

BOOST_AUTO_TEST_CASE(LN)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("LN(1) == 0");
  i = i.replace(f_ln, {1}, l0 + 0);  // [0] LN 1
  i = i.replace(  c1,  {}, l0 + 1);  // [1] 1
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("LN(0) == nan");
  i = i.replace(f_ln, {1}, l0 + 0);  // [0] LN 1
  i = i.replace(  c0,  {}, l0 + 1);  // [1] 0
  ret = vita::interpreter(i).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i);

  BOOST_TEST_CHECKPOINT("LN(Z) = std::log(Z)");
  i = i.replace(f_ln, {1}, l0 + 0);  // [0] LN 1
  i = i.replace(   z,  {}, l0 + 1);  // [1] Z
  for (unsigned j(0); j < 1000; ++j)
  {
    static_pointer_cast<Z>(z)->val = vita::random::between<double>(0.1,
                                                                   1000000.0);
    ret = vita::interpreter(i).run();
    BOOST_REQUIRE_MESSAGE(vita::any_cast<double>(ret) ==
                          std::log(static_pointer_cast<Z>(z)->val), "\n" << i);
  }
}

BOOST_AUTO_TEST_SUITE_END()
