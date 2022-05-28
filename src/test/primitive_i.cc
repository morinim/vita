/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013-2022 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/mep/interpreter.h"
#include "kernel/random.h"

#include "test/fixture4.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("PRIMITIVE_I")
{

TEST_CASE_FIXTURE(fixture4, "i_add")
{
  using namespace vita;

  const i_mep i1({
                  {{i_add, {1, 2}}},  // [0] ADD 1,2
                  {{   c0,   null}},  // [1] 0
                  {{    x,   null}}   // [2] X
                 });
  ret = run(i1);
  INFO(i1);
  CHECK(std::get<D_INT>(ret) == x_val);

  // ADD(X,Y) == X+Y"
  const i_mep i2({
                   {{i_add, {1, 2}}},  // [0] ADD 1,2
                   {{    y,   null}},  // [1] Y
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i2);
  INFO(i2);
  CHECK(std::get<D_INT>(ret) == x_val + y_val);

  // ADD(X,-X) == 0
  const i_mep i3({
                   {{i_add, {1, 2}}},  // [0] ADD 1,2
                   {{    x,   null}},  // [1] X
                   {{neg_x,   null}}   // [2] -X
                 });
  ret = run(i3);
  INFO(i3);
  CHECK(std::get<D_INT>(ret) == 0);

  // ADD(X,Y) == ADD(Y,X)
  const i_mep i4({
                   {{i_sub, {1, 2}}},  // [0] SUB 1,2
                   {{i_add, {3, 4}}},  // [1] ADD 3,4
                   {{i_add, {4, 3}}},  // [2] ADD 4,3
                   {{    x,   null}},  // [3] X
                   {{    y,   null}}  // [4] Y
                 });
  ret = run(i4);
  INFO(i4);
  CHECK(std::get<D_INT>(ret) == 0);
}

TEST_CASE_FIXTURE(fixture4, "i_div")
{
  using namespace vita;

  // DIV(X,X) == 1
  const i_mep i1({
                   {{i_div, {1, 2}}},  // [0] DIV 1, 2
                   {{    x,   null}},  // [1] X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i1);
  INFO(i1);
  CHECK(std::get<D_INT>(ret) == 1);

  // DIV(X,1) == X
  const i_mep i2({
                   {{i_div, {1, 2}}},  // [0] DIV 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c1,   null}}   // [2] 1
                 });
  ret = run(i2);
  INFO(i2);
  CHECK(std::get<D_INT>(ret) == x_val);

  // DIV(-X,X) == -1
  const i_mep i3({
                   {{i_div, {1, 2}}},  // [0] DIV 1, 2
                   {{neg_x,   null}},  // [1] -X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i3);
  INFO(i3);
  CHECK(std::get<D_INT>(ret) == -1);

  // DIV(X,0) == X
  const i_mep i4({
                   {{i_div, {1, 2}}},  // [0] DIV 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0
                 });
  ret = run(i4);
  INFO(i4);
  CHECK(std::get<D_INT>(ret) == x_val);
}

TEST_CASE_FIXTURE(fixture4, "i_ife")
{
  using namespace vita;
  using i_interp = vita::interpreter<vita::i_mep>;

  // IFE(0,0,1,0) == 1
  const i_mep i1({
                   {{i_ife, {1, 1, 2, 1}}},  // [0] IFE 1,1,2,1
                   {{   c0,         null}},  // [1] 0
                   {{   c1,         null}}   // [2] 1
                 });
  ret = run(i1);
  INFO(i1);
  CHECK(std::get<D_INT>(ret) == 1);

  // IFE(0,1,1,0) == 0
  const i_mep i2({
                   {{i_ife, {1, 2, 2, 1}}},  // [0] IFE 1,2,2,1
                   {{   c0,         null}},  // [1] 0
                   {{   c1,         null}}   // [2] 1
                 });
  ret = run(i2);
  INFO(i2);
  CHECK(std::get<D_INT>(ret) == 0);

  // IFE(Z,X,1,0) == 0
  const i_mep i3({
                   {{i_ife, {1, 2, 3, 4}}},  // [0] IFE Z, X, 1, 0
                   {{    z,         null}},  // [1] Z
                   {{    x,         null}},  // [2] X
                   {{   c1,         null}},  // [2] 1
                   {{   c0,         null}}   // [1] 0
                 });
  static_cast<Z *>(z)->val = 0;
  ret = run(i3);
  INFO(i3);
  CHECK(std::get<D_INT>(ret) == 0);

  // IFE SAME TERM COMPARISON PENALTY
  CHECK(i_interp(&i1).penalty() > 0);

  // IFE NO PENALTY
  CHECK(i_interp(&i2).penalty() == doctest::Approx(0.0));
  CHECK(i_interp(&i3).penalty() == doctest::Approx(0.0));

  // IFE SAME RESULT PENALTY
  const i_mep i4({
                   {{i_ife, {1, 2, 2, 2}}},  // [0] IFE 1,2,2,2
                   {{   c0,         null}},  // [1] 0
                   {{   c1,         null}}   // [2] 1
                 });
  CHECK(i_interp(&i4).penalty() > 0);
}

TEST_CASE_FIXTURE(fixture4, "i_mul")
{
  using namespace vita;

  // MUL(X,0) == 0
  const i_mep i1({
                   {{i_mul, {1, 2}}},  // [0] MUL 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0
                 });
  ret = run(i1);
  INFO(i1);
  CHECK(std::get<D_INT>(ret) == 0);

  // MUL(X,1) == X
  const i_mep i2({
                   {{i_mul, {1, 2}}},  // [0] MUL 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c1,   null}}   // [2] 1
                 });
  ret = run(i2);
  INFO(i2);
  CHECK(std::get<D_INT>(ret) == x_val);

  // MUL(X,2) == ADD(X,X)
  const i_mep i3({
                   {{i_sub, {1, 2}}},  // [0] SUB 1, 2
                   {{i_add, {3, 3}}},  // [1] ADD 3, 3
                   {{i_mul, {3, 4}}},  // [2] MUL 3, 4
                   {{    x,   null}},  // [3] X
                   {{   c2,   null}}   // [4] 2
                 });
  ret = run(i3);
  INFO(i3);
  CHECK(std::get<D_INT>(ret) == 0);
}

TEST_CASE_FIXTURE(fixture4, "i_sub")
{
  using namespace vita;

  // SUB(X,-X) == 0
  const i_mep i1({
                   {{i_sub, {1, 2}}},  // [0] SUB 1, 2
                   {{    x,   null}},  // [1] X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i1);
  INFO(i1);
  CHECK(std::get<D_INT>(ret) == 0);

  // SUB(X,0) == X
  const i_mep i2({
                   {{i_sub, {1, 2}}},  // [0] SUB 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0
                 });
  ret = run(i2);
  INFO(i2);
  CHECK(std::get<D_INT>(ret) == x_val);

  // SUB(Z,X) == Z-X
  const i_mep i3({
                   {{i_sub, {1, 2}}},  // [0] SUB 1, 2
                   {{    z,   null}},  // [1] Z
                   {{    x,   null}}   // [2] X
                 });
  for (unsigned j(0); j < 1000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between<int>(-1000, 1000);
    ret = run(i3);
    INFO(i3);
    CHECK(std::get<D_INT>(ret) == static_cast<Z *>(z)->val - x_val);
  }
}

}  // TEST_SUITE("PRIMITIVE_I")
