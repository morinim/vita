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
#include "kernel/primitive/sr_pri.h"

#define BOOST_TEST_MODULE Primitive
#include "boost/test/unit_test.hpp"

using namespace boost;

typedef std::shared_ptr<vita::sr::constant> constant_ptr;

struct F
{
  F()
    : c0(new vita::sr::constant(0)),
      c1(new vita::sr::constant(1)),
      c2(new vita::sr::constant(2)),
      c3(new vita::sr::constant(3)),
      x(new vita::sr::constant(123)),
      neg_x(new vita::sr::constant(-123)),
      y(new vita::sr::constant(321)),
      f_abs(new vita::sr::abs()),
      f_add(new vita::sr::add()),
      f_div(new vita::sr::div()),
      f_idiv(new vita::sr::idiv()),
      f_mul(new vita::sr::mul()),
      f_sub(new vita::sr::sub())
  {
    BOOST_TEST_MESSAGE("Setup fixture");
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

  constant_ptr c0;
  constant_ptr c1;
  constant_ptr c2;
  constant_ptr c3;
  constant_ptr x;
  constant_ptr neg_x;
  constant_ptr y;

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
  // with assign::list_of(1).
  BOOST_TEST_CHECKPOINT("ABS(-X) == X");
  i = i.replace(f_abs, {1}, 0);  // [0] ABS 1
  i = i.replace(neg_x,  {}, 1);  // [1] -X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);

  BOOST_TEST_CHECKPOINT("ABS(X) == X");
  i = i.replace(f_abs, {1}, 0);  // [0] ABS 1
  i = i.replace(    x,  {}, 1);  // [1] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);
}

BOOST_AUTO_TEST_CASE(ADD)
{
  vita::individual i(env, true);

  BOOST_TEST_CHECKPOINT("ADD(X,0) == X");
  i = i.replace(f_add, {1, 2}, 0);  // [0] ADD 1,2
  i = i.replace(   c0,     {}, 1);  // [1] 0
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y)=X+Y");
  i = i.replace(f_add, {1, 2}, 0);  // [0] ADD 1,2
  i = i.replace(    y,     {}, 1);  // [1] Y
  i = i.replace(    x,     {}, 2);  // [2] X
  ret = (vita::interpreter(i))();
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == y->val+x->val, "\n" << i);

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
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);

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
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);

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
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);

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
  BOOST_REQUIRE_MESSAGE(any_cast<double>(ret) == x->val, "\n" << i);
}

BOOST_AUTO_TEST_SUITE_END()
