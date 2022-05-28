/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/random.h"
#include "kernel/gp/mep/i_mep.h"

#include "test/fixture3.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("PRIMITIVE_D")
{

TEST_CASE_FIXTURE(fixture3, "f_abs")
{
  using namespace vita;

  // ABS(-X) == X
  const i_mep i1({
                   {{f_abs,  {1}}},  // [0] FABS [1]
                   {{neg_x, null}}   // [1] -X
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(x_val));

  // ABS(X) == X
  const i_mep i2({
                   {{f_abs,  {1}}},  // [0] FABS [1]
                   {{    x, null}}   // [1] X
                 });
  ret = run(i2);
  CHECK(real::base(ret) == doctest::Approx(x_val));
}

TEST_CASE_FIXTURE(fixture3, "f_add")
{
  using namespace vita;

  // ADD(X,0) == X
  const i_mep i1({
                   {{f_add, {1, 2}}},  // [0] FADD [1], [2]
                   {{   c0,   null}},  // [1] 0.0
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(x_val));

  // ADD(X,Y) == X+Y
  const i_mep i2({
                   {{f_add, {1, 2}}},  // [0] FADD [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = run(i2);
  const auto res1(real::base(ret));
  CHECK(doctest::Approx(res1) == y_val + x_val);

  // ADD(X,-X) == 0
  const i_mep i3({
                   {{f_add, {1, 2}}},  // [0] FADD [1], [2]
                   {{    x,   null}},  // [1] X
                   {{neg_x,   null}}   // [2] -X
                 });
  ret = run(i3);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // ADD(X,Y) == ADD(Y,X)
  const i_mep i4({
                   {{f_add, {2, 1}}},  // [0] FADD [2], [1]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = run(i4);
  CHECK(real::base(ret) == doctest::Approx(res1));
}

TEST_CASE_FIXTURE(fixture3, "f_aq")
{
  using namespace vita;

  const auto rx(x_val);

  // AQ(X,X) == 1
  const i_mep i1({
                   {{f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{   x,   null}},  // [1] X
                   {{   x,   null}}   // [2] X
                 });
  ret = run(i1);
  const auto res1(real::base(ret));
  CHECK(doctest::Approx(res1) == rx / std::sqrt(1.0 + std::pow(rx, 2.0)));

  // AQ(Z,1) == Z/SQRT(2)
  const i_mep i2({
                   {{f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{   z,   null}},  // [1] Z
                   {{  c1,   null}}   // [2] 1.0
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000000.0, 1000000.0);
    ret = run(i2);

    const auto rz(static_cast<Z *>(z)->val);
    CHECK(doctest::Approx(real::base(ret)) == rz / std::sqrt(2.0));
  }

  // AQ(-X,X) == -AQ(X,X)
  const i_mep i3({
                   {{ f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{neg_x,   null}},  // [1] -X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i3);
  CHECK(real::base(ret) == doctest::Approx(-res1));

  // AQ(X,0) == X
  const i_mep i4({
                   {{f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{   x,   null}},  // [1] X
                   {{  c0,   null}}   // [2] 0.0
                 });
  ret = run(i4);
  CHECK(real::base(ret) == doctest::Approx(rx));

  // AQ(0,X) == 0
  const i_mep i5({
                   {{f_aq, {2, 1}}},  // [0] FAQ [2], [1]
                   {{   x,   null}},  // [1] X
                   {{  c0,   null}}   // [2] 0.0
                 });
  ret = run(i5);
  CHECK(real::base(ret) == doctest::Approx(0.0));
}

TEST_CASE_FIXTURE(fixture3, "f_cos")
{
  using namespace vita;

  // COS(Z) == std::cos(Z)
  const i_mep i1({
                   {{f_cos,  {1}}},  // [0] FCOS [1]
                   {{    z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.0, 1000000.0);
    ret = run(i1);
    CHECK(doctest::Approx(real::base(ret))
          == std::cos(static_cast<Z *>(z)->val));
  }

  // COS(0) == 1
  const i_mep i2({
                   {{f_cos,  {1}}},  // [0] FCOS [1]
                   {{   c0, null}}   // [1] 0
                 });
  ret = run(i2);
  CHECK(real::base(ret) == doctest::Approx(1.0));
}

TEST_CASE_FIXTURE(fixture3, "f_div")
{
  using namespace vita;

  // DIV(X,X) == 1
  const i_mep i1({
                   {{f_div, {1, 2}}},  // [0] FDIV [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // DIV(Z,1) == Z
  const i_mep i2({
                   {{f_div, {1, 2}}},  // [0] FDIV [1], [2]
                   {{    z,   null}},  // [1] Z
                   {{   c1,   null}}   // [2] 1.0
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000000.0, 1000000.0);
    ret = run(i2);
    CHECK(doctest::Approx(real::base(ret)) == static_cast<Z *>(z)->val);
  }

  // DIV(-X,X) == -1
  const i_mep i3({
                   {{f_div, {1, 2}}},  // [0] FDIV 1, 2
                   {{neg_x,   null}},  // [1] -X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i3);
  CHECK(real::base(ret) == doctest::Approx(-1.0));

  // DIV(X,0) == NaN
  const i_mep i4({
                   {{f_div, {1, 2}}},  // [0] FDIV [1], [2]
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = run(i4);
  CHECK(!has_value(ret));
}

TEST_CASE_FIXTURE(fixture3, "f_idiv")
{
  using namespace vita;

  // IDIV(X,X) == 1
  const i_mep i1({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{     x,   null}},  // [1] X
                   {{     x,   null}}   // [2] X
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // IDIV(X,1) == X
  const i_mep i2({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{     x,   null}},  // [1] X
                   {{    c1,   null}}   // [2] 1.0
                 });
  ret = run(i2);
  CHECK(doctest::Approx(real::base(ret)) == x_val);

  // IDIV(-X,X) == -1
  const i_mep i3({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{ neg_x,   null}},  // [1] -X
                   {{     x,   null}}   // [2] X
                 });
  ret = run(i3);
  CHECK(real::base(ret) == doctest::Approx(-1.0));

  // IDIV(3,2) == 1
  const i_mep i4({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{    c3,   null}},  // [1] 3.0
                   {{    c2,   null}}   // [2] 2.0
                 });
  ret = run(i4);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // IDIV(X,0) == NaN
  const i_mep i5({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{     x,   null}},  // [1] X
                   {{    c0,   null}}   // [2] 0.0
                 });
  ret = run(i5);
  CHECK(!has_value(ret));
}

TEST_CASE_FIXTURE(fixture3, "f_ife")
{
  using namespace vita;
  using i_interp = vita::interpreter<vita::i_mep>;

  // IFE(0,0,1,0) == 1
  const i_mep i1({
                   {{f_ife, {1, 1, 2, 1}}},  // [0] FIFE [1], [1], [2], [1]
                   {{   c0,         null}},  // [1] 0.0
                   {{   c1,         null}}   // [2] 1.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // IFE(0,1,1,0) == 0
  const i_mep i2({
                   {{f_ife, {1, 2, 2, 1}}},  // [0] FIFE [1], [2], [2], [1]
                   {{   c0,         null}},  // [1] 0.0
                   {{   c1,         null}}   // [2] 1.0
                 });
  ret = run(i2);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // IFE(Z,X,1,0) == 0
  const i_mep i3({
                   {{f_ife, {1, 2, 3, 4}}},  // [0] FIFE Z, X, 1.0, 0.0
                   {{    z,         null}},  // [1] Z
                   {{    x,         null}},  // [2] X
                   {{   c1,         null}},  // [3] 1.0
                   {{   c0,         null}}   // [4] 0.0
                 });
  static_cast<Z *>(z)->val = 0;
  ret = run(i3);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // IFE SAME TERM COMPARISON PENALTY");
  CHECK(i_interp(&i1).penalty() > 0.0);

  // IFE NO PENALTY
  CHECK(i_interp(&i2).penalty() == doctest::Approx(0.0));
  CHECK(i_interp(&i3).penalty() == doctest::Approx(0.0));

  // IFE SAME RESULT PENALTY
  const i_mep i4({
                   {{f_ife, {1, 2, 2, 2}}},  // [0] FIFE [1], [2], [2], [2]
                   {{   c0,         null}},  // [1] 0.0
                   {{   c1,         null}}   // [2] 1.0
                 });
  CHECK(i_interp(&i4).penalty() > doctest::Approx(0.0));
}

TEST_CASE_FIXTURE(fixture3, "f_ifz")
{
  using namespace vita;

  // IFZ(0,1,0) == 1
  const i_mep i1({
                   {{f_ifz, {1, 2, 1}}},  // [0] FIFZ [1], [2], [1]
                   {{   c0,      null}},  // [1] 0.0
                   {{   c1,      null}}   // [2] 1.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // IFE(1,1,0) == 0
  const i_mep i2({
                   {{f_ifz, {2, 2, 1}}},  // [0] FIFZ [2], [2], [1]
                   {{   c0,      null}},  // [1] 0.0
                   {{   c1,      null}}   // [2] 1.0
                 });
  ret = run(i2);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // IFZ(Z,Z,Z-Z) == 0
  const i_mep i3({
                   {{f_ifz, {2, 2, 1}}},  // [0] FIFZ Z, Z, [1]
                   {{f_sub,    {2, 2}}},  // [1] FSUB Z, Z
                   {{    z,      null}}   // [2] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000000.0, 1000000.0);
    ret = run(i3);

    CHECK(real::base(ret) == doctest::Approx(0.0));
  }
}

TEST_CASE_FIXTURE(fixture3, "f_max")
{
  using namespace vita;

  // MAX(0,0) == 0
  const i_mep i1({
                   {{f_max, {1, 2}}},  // [0] FMAX [1], [2]
                   {{   c0,   null}},  // [1] 0.0
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // MAX(0,1) == 1
  const i_mep i2({
                   {{f_max, {1, 2}}},  // [0] FMAX [1], [2]
                   {{   c0,   null}},  // [1] 0.0
                   {{   c1,   null}}   // [2] 1.0
                 });
  ret = run(i2);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // MAX(X,Y) == (X>Y ? X : Y)
  const i_mep i3({
                   {{f_max, {1, 2}}},  // [0] FMAX [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = run(i3);
  const auto res1(real::base(ret));
  CHECK(doctest::Approx(res1) == std::max(x_val, y_val));

  // MAX(X,Y) == MAX(Y,X)
  const i_mep i4({
                   {{f_max, {2, 1}}},  // [0] FMAX [2], [1]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = run(i4);
  CHECK(real::base(ret) == doctest::Approx(res1));
}

TEST_CASE_FIXTURE(fixture3, "f_mul")
{
  using namespace vita;

  // MUL(X,0) == 0
  const i_mep i1({
                   {{f_mul, {1, 2}}},  // [0] FMUL [1], [2]
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // MUL(X,1) == X
  const i_mep i2({
                   {{f_mul, {1, 2}}},  // [0] FMUL 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c1,   null}}   // [2] 1.0
                 });
  ret = run(i2);
  CHECK(doctest::Approx(real::base(ret)) == x_val);

  // MUL(X,2) == ADD(X,X)
  const i_mep i3({
                   {{f_sub, {1, 2}}},  // [0] FSUB 1, 2
                   {{f_add, {3, 3}}},  // [1] FADD 3, 3
                   {{f_mul, {3, 4}}},  // [2] FMUL 3, 4
                   {{    x,   null}},  // [3] X
                   {{   c2,   null}}   // [4] 2.0
                 });
  ret = run(i3);
  CHECK(real::base(ret) == doctest::Approx(0.0));
}

TEST_CASE_FIXTURE(fixture3, "f_sin")
{
  using namespace vita;

  // SIN(Z) == std::sin(Z)
  const i_mep i1({
                   {{f_sin,  {1}}},  // [0] FSIN [1]
                   {{    z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.0, 1000000.0);
    ret = run(i1);
    CHECK(doctest::Approx(real::base(ret))
          == std::sin(static_cast<Z *>(z)->val));
  }

  // SIN(0) == 0
  const i_mep i2({
                   {{f_sin,  {1}}},  // [0] FCOS [1]
                   {{   c0, null}}   // [1] 0
                 });
  ret = run(i2);
  CHECK(real::base(ret) == doctest::Approx(0.0));
}

TEST_CASE_FIXTURE(fixture3, "f_sqrt")
{
  using namespace vita;

  // SQRT(1) == 1
  const i_mep i1({
                   {{f_sqrt,  {1}}},  // [0] SQRT [1]
                   {{    c1, null}}   // [1] 1.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(1.0));

  // SQRT(-X) == NaN
  const i_mep i2({
                   {{f_sqrt,  {1}}},  // [0] FSQRT [1]
                   {{neg_x,  null}}   // [1] -X
                 });
  ret = run(i2);
  CHECK(!has_value(ret));

  // SQRT(Z) == std::sqrt(Z)
  const i_mep i3({
                   {{f_sqrt,  {1}}},  // [0] FSQRT [1]
                   {{     z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.0, 1000000.0);
    ret = run(i3);
    CHECK(doctest::Approx(real::base(ret))
          == std::sqrt(static_cast<Z *>(z)->val));
  }
}

TEST_CASE_FIXTURE(fixture3, "f_sub")
{
  using namespace vita;

  // SUB(X,-X) == 0
  const i_mep i1({
                   {{f_sub, {1, 2}}},  // [0] FSUB [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    x,   null}}   // [2] X
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // SUB(X,0) == X
  const i_mep i2({
                   {{f_sub, {1, 2}}},  // [0] FSUB [1], [2]
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = run(i2);
  CHECK(doctest::Approx(real::base(ret)) == x_val);

  // SUB(Z,X) == Z-X
  const i_mep i3({
                   {{f_sub, {1, 2}}},  // [0] FSUB [1], [2]
                   {{    z,   null}},  // [1] Z
                   {{    x,   null}}   // [2] X
                 });
  for (unsigned j(0); j < 1000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000.0, 1000.0);
    ret = run(i3);

    const auto v1(real::base(ret));
    const auto v2(static_cast<Z *>(z)->val - x_val);
    CHECK(v1 == doctest::Approx(v2));
  }
}

TEST_CASE_FIXTURE(fixture3, "f_ln")
{
  using namespace vita;

  // LN(1) == 0
  const i_mep i1({
                   {{f_ln,  {1}}},  // [0] FLN [1]
                   {{  c1, null}}   // [1] 1.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(0.0));

  // LN(0) == NaN
  const i_mep i2({
                   {{f_ln,  {1}}},  // [0] FLN [1]
                   {{  c0, null}}   // [1] 0.0
                 });
  ret = run(i2);
  CHECK(!has_value(ret));

  // LN(Z) == std::log(Z)
  const i_mep i3({
                   {{f_ln,  {1}}},  // [0] LN 1
                   {{   z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.1, 1000000.0);
    ret = run(i3);
    CHECK(doctest::Approx(real::base(ret))
          == std::log(static_cast<Z *>(z)->val));
  }
}

TEST_CASE_FIXTURE(fixture3, "f_sigmoid")
{
  using namespace vita;

  // SIGMOID(0) == 0.5
  const i_mep i1({
                   {{f_sigmoid,  {1}}},  // [0] FSIGMOID [1]
                   {{       c0, null}}   // [1] 0.0
                 });
  ret = run(i1);
  CHECK(real::base(ret) == doctest::Approx(0.5));

  // SIGMOID(X) == 1.0 / (1.0 + std::exp(-X))
  const i_mep i2({
                   {{f_sigmoid,  {1}}},  // [0] FSIGMOID [1]
                   {{        z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 200; ++j)
  {
    const auto rx(vita::random::between(-100.0, 100.0));
    static_cast<Z *>(z)->val = rx;
    ret = run(i2);

    if (has_value(ret))
    {
      const auto expected(1.0 / (1.0 + std::exp(-rx)));
      CHECK(doctest::Approx(real::base(ret)) == expected);
    }
  }
}

}  // TEST_SUITE("PRIMITIVE_D")
