/**
 *
 *  \file test_primitive.cc
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
#include <iostream>

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"
#include "kernel/primitive/factory.h"

#define BOOST_TEST_MODULE Primitive
#include "boost/test/unit_test.hpp"

using namespace boost;

struct F
{
  F()
  {
    BOOST_TEST_MESSAGE("Setup fixture");

    static vita::symbol_factory &factory(vita::symbol_factory::instance());

    c0 = factory.make("0", vita::d_double, 0);
    c1 = factory.make("1", vita::d_double, 0);
    c2 = factory.make("2", vita::d_double, 0);
    c3 = factory.make("3", vita::d_double, 0);
    x = factory.make("123", vita::d_double, 0);
    neg_x = factory.make("-123", vita::d_double, 0);
    y = factory.make("321", vita::d_double, 0);
    f_abs = factory.make("ABS", vita::d_double, 0);
    f_add = factory.make("ADD", vita::d_double, 0);
    f_div = factory.make("DIV", vita::d_double, 0);
    f_idiv = factory.make("IDIV", vita::d_double, 0);
    f_mul = factory.make("MUL", vita::d_double, 0);
    f_sub = factory.make("SUB", vita::d_double, 0);

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

  vita::symbol_ptr f_abs;
  vita::symbol_ptr f_add;
  vita::symbol_ptr f_div;
  vita::symbol_ptr f_idiv;
  vita::symbol_ptr f_mul;
  vita::symbol_ptr f_sub;

  vita::environment env;
  any ret;
};

BOOST_FIXTURE_TEST_SUITE(SymbolicRegressionSymbols, F)

BOOST_AUTO_TEST_CASE(ABS)
{
  vita::individual i(env, true);

  // With a not C++11 conformant compiler the {1} expression can be changed
  // with boost::assign::list_of(1).
  BOOST_TEST_CHECKPOINT("ABS(-X) == X");
  i = i.replace(f_abs, {1}, 0);  // [0] ABS 1
  i = i.replace(neg_x,  {}, 1);  // [1] -X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("ABS(X) == X");
  i = i.replace(f_abs, {1}, 0);  // [0] ABS 1
  i = i.replace(    x,  {}, 1);  // [1] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);
}

BOOST_AUTO_TEST_CASE(ADD)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("ADD(X,0) == X");
  i = i.replace(f_add, {1, 2}, 0);  // [0] ADD 1,2
  i = i.replace(   c0,     {}, 1);  // [1] 0
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y)=X+Y");
  i = i.replace(f_add, {1, 2}, 0);  // [0] ADD 1,2
  i = i.replace(    y,     {}, 1);  // [1] Y
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(y->eval(0)) +
                        any_cast<double>(x->eval(0)), "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,-X) == 0");
  i = i.replace(f_add, {1, 2}, 0);  // [0] ADD 1,2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(neg_x,     {}, 2);  // [2] -X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == ADD(Y,X)");
  i = i.replace(f_sub, {1, 2}, 0);  // [0] SUB 1,2
  i = i.replace(f_add, {3, 4}, 1);  // [1] ADD 3,4
  i = i.replace(f_add, {4, 3}, 2);  // [2] ADD 4,3
  i = i.replace(    x,     {}, 3);  // [3] X
  i = i.replace(    y,     {}, 4);  // [4] Y
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(DIV)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("DIV(X,X) == 1");
  i = i.replace(f_div, {1, 2}, 0);  // [0] DIV 1, 2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(X,1) == X");
  i = i.replace(f_div, {1, 2}, 0);  // [0] DIV 1, 2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(   c1,     {}, 2);  // [2] 1
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(-X,X) == -1");
  i = i.replace(f_div, {1, 2}, 0);  // [0] DIV 1, 2
  i = i.replace(neg_x,     {}, 1);  // [1] -X
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == -1, "\n" << i);
}

BOOST_AUTO_TEST_CASE(IDIV)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("IDIV(X,X) == 1");
  i = i.replace(f_idiv, {1, 2}, 0);  // [0] DIV 1, 2
  i = i.replace(     x,     {}, 1);  // [1] X
  i = i.replace(     x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(X,1) == X");
  i = i.replace(f_idiv, {1, 2}, 0);  // [0] DIV 1, 2
  i = i.replace(     x,     {}, 1);  // [1] X
  i = i.replace(    c1,     {}, 2);  // [2] 1
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(-X,X) == -1");
  i = i.replace(f_idiv, {1, 2}, 0);  // [0] IDIV 1, 2
  i = i.replace( neg_x,     {}, 1);  // [1] -X
  i = i.replace(     x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == -1, "\n" << i);

  BOOST_TEST_CHECKPOINT("IDIV(3,2) == 1");
  i = i.replace(f_idiv, {1, 2}, 0);  // [0] IDIV 1, 2
  i = i.replace(    c3,     {}, 1);  // [1] 3
  i = i.replace(    c2,     {}, 2);  // [2] 2
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 1, "\n" << i);
}

BOOST_AUTO_TEST_CASE(MUL)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("MUL(X,0) == 0");
  i = i.replace(f_mul, {1, 2}, 0);  // [0] MUL 1, 2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(   c0,     {}, 2);  // [2] 0
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("MUL(X,1) == X");
  i = i.replace(f_mul, {1, 2}, 0);  // [0] MUL 1, 2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(   c1,     {}, 2);  // [2] 1
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("MUL(X,2) == ADD(X,X)");
  i = i.replace(f_sub, {1, 2}, 0);  // [0] SUB 1, 2
  i = i.replace(f_add, {3, 3}, 1);  // [1] ADD 3, 3
  i = i.replace(f_mul, {3, 4}, 2);  // [2] MUL 3, 4
  i = i.replace(    x,     {}, 3);  // [3] X
  i = i.replace(   c2,     {}, 4);  // [4] 2
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 0.0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(SUB)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("SUB(X,-X) == 0");
  i = i.replace(f_sub, {1, 2}, 0);  // [0] SUB 1, 2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("SUB(X,0) == X");
  i = i.replace(f_sub, {1, 2}, 0);  // [0] SUB 1, 2
  i = i.replace(    x,     {}, 1);  // [1] X
  i = i.replace(   c0,     {}, 2);  // [2] 0
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == any_cast<double>(x->eval(0)),
                        "\n" << i);
}

BOOST_AUTO_TEST_SUITE_END()
