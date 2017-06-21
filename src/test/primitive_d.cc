/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/i_mep.h"
#include "kernel/random.h"
#include "kernel/src/primitive/real.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE primitive
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr vita::real::base_t epsilon(0.00001);

#include "factory_fixture3.h"
#endif

BOOST_FIXTURE_TEST_SUITE(primitive_d, F_FACTORY3)

BOOST_AUTO_TEST_CASE(ABS, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("ABS(-X) == X");
  const i_mep i1({
                   {{f_abs,  {1}}},  // [0] FABS [1]
                   {{neg_x, null}}   // [1] -X
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("ABS(X) == X");
  const i_mep i2({
                   {{f_abs,  {1}}},  // [0] FABS [1]
                   {{    x, null}}   // [1] X
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == real::base(x->eval(nullptr)));
}

BOOST_AUTO_TEST_CASE(ADD, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("ADD(X,0) == X");
  const i_mep i1({
                   {{f_add, {1, 2}}},  // [0] FADD [1], [2]
                   {{   c0,   null}},  // [1] 0.0
                   {{    x,   null}}   // [2] X
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == X+Y");
  const i_mep i2({
                   {{f_add, {1, 2}}},  // [0] FADD [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = i_interp(&i2).run();
  const auto res1(real::base(ret));
  BOOST_TEST(res1 ==
             real::base(y->eval(nullptr)) + real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("ADD(X,-X) == 0");
  const i_mep i3({
                   {{f_add, {1, 2}}},  // [0] FADD [1], [2]
                   {{    x,   null}},  // [1] X
                   {{neg_x,   null}}   // [2] -X
                 });
  ret = i_interp(&i3).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == ADD(Y,X)");
  const i_mep i4({
                   {{f_add, {2, 1}}},  // [0] FADD [2], [1]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = i_interp(&i4).run();
  BOOST_TEST(real::base(ret) == res1);
}

BOOST_AUTO_TEST_CASE(AQ, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  const auto rx(real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("AQ(X,X) == 1");
  const i_mep i1({
                   {{f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{   x,   null}},  // [1] X
                   {{   x,   null}}   // [2] X
                 });
  ret = i_interp(&i1).run();
  const auto res1(real::base(ret));
  BOOST_TEST(res1 == rx / std::sqrt(1.0 + std::pow(rx, 2.0)));

  BOOST_TEST_CHECKPOINT("AQ(Z,1) == Z/SQRT(2)");
  const i_mep i2({
                   {{f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{   z,   null}},  // [1] Z
                   {{  c1,   null}}   // [2] 1.0
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000000.0, 1000000.0);
    ret = i_interp(&i2).run();

    const auto rz(static_cast<Z *>(z)->val);
    BOOST_TEST(real::base(ret) == rz / std::sqrt(2.0));
  }

  BOOST_TEST_CHECKPOINT("AQ(-X,X) == -AQ(X,X)");
  const i_mep i3({
                   {{ f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{neg_x,   null}},  // [1] -X
                   {{    x,   null}}   // [2] X
                 });
  ret = i_interp(&i3).run();
  BOOST_TEST(real::base(ret) == -res1);

  BOOST_TEST_CHECKPOINT("AQ(X,0) == X");
  const i_mep i4({
                   {{f_aq, {1, 2}}},  // [0] FAQ [1], [2]
                   {{   x,   null}},  // [1] X
                   {{  c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i4).run();
  BOOST_TEST(real::base(ret) == rx);

  BOOST_TEST_CHECKPOINT("AQ(0,X) == 0");
  const i_mep i5({
                   {{f_aq, {2, 1}}},  // [0] FAQ [2], [1]
                   {{   x,   null}},  // [1] X
                   {{  c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i5).run();
  BOOST_TEST(real::base(ret) == 0.0);
}

BOOST_AUTO_TEST_CASE(DIV, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("DIV(X,X) == 1");
  const i_mep i1({
                   {{f_div, {1, 2}}},  // [0] FDIV [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    x,   null}}   // [2] X
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("DIV(Z,1) == Z");
  const i_mep i2({
                   {{f_div, {1, 2}}},  // [0] FDIV [1], [2]
                   {{    z,   null}},  // [1] Z
                   {{   c1,   null}}   // [2] 1.0
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000000.0, 1000000.0);
    ret = i_interp(&i2).run();
    BOOST_TEST(real::base(ret) == static_cast<Z *>(z)->val);
  }

  BOOST_TEST_CHECKPOINT("DIV(-X,X) == -1");
  const i_mep i3({
                   {{f_div, {1, 2}}},  // [0] FDIV 1, 2
                   {{neg_x,   null}},  // [1] -X
                   {{    x,   null}}   // [2] X
                 });
  ret = i_interp(&i3).run();
  BOOST_TEST(real::base(ret) == -1.0);

  BOOST_TEST_CHECKPOINT("DIV(X,0) == NaN");
  const i_mep i4({
                   {{f_div, {1, 2}}},  // [0] FDIV [1], [2]
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i4).run();
  BOOST_TEST(ret.empty());
}

BOOST_AUTO_TEST_CASE(IDIV, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("IDIV(X,X) == 1");
  const i_mep i1({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{     x,   null}},  // [1] X
                   {{     x,   null}}   // [2] X
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("IDIV(X,1) == X");
  const i_mep i2({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{     x,   null}},  // [1] X
                   {{    c1,   null}}   // [2] 1.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("IDIV(-X,X) == -1");
  const i_mep i3({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{ neg_x,   null}},  // [1] -X
                   {{     x,   null}}   // [2] X
                 });
  ret = i_interp(&i3).run();
  BOOST_TEST(real::base(ret) == -1.0);

  BOOST_TEST_CHECKPOINT("IDIV(3,2) == 1");
  const i_mep i4({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{    c3,   null}},  // [1] 3.0
                   {{    c2,   null}}   // [2] 2.0
                 });
  ret = i_interp(&i4).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("IDIV(X,0) == NaN");
  const i_mep i5({
                   {{f_idiv, {1, 2}}},  // [0] IDIV [1], [2]
                   {{     x,   null}},  // [1] X
                   {{    c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i5).run();
  BOOST_TEST(ret.empty());
}

BOOST_AUTO_TEST_CASE(IFE, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("IFE(0,0,1,0) == 1");
  const i_mep i1({
                   {{f_ife, {1, 1, 2, 1}}},  // [0] FIFE [1], [1], [2], [1]
                   {{   c0,         null}},  // [1] 0.0
                   {{   c1,         null}}   // [2] 1.0
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("IFE(0,1,1,0) == 0");
  const i_mep i2({
                   {{f_ife, {1, 2, 2, 1}}},  // [0] FIFE [1], [2], [2], [1]
                   {{   c0,         null}},  // [1] 0.0
                   {{   c1,         null}}   // [2] 1.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("IFE(Z,X,1,0) == 0");
  const i_mep i3({
                   {{f_ife, {1, 2, 3, 4}}},  // [0] FIFE Z, X, 1.0, 0.0
                   {{    z,         null}},  // [1] Z
                   {{    x,         null}},  // [2] X
                   {{   c1,         null}},  // [3] 1.0
                   {{   c0,         null}}   // [4] 0.0
                 });
  static_cast<Z *>(z)->val = 0;
  ret = i_interp(&i3).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("IFE SAME COMPARISON-TERM PENALTY");
  auto penalty(i_interp(&i1).penalty());
  BOOST_TEST(penalty > 0.0);

  BOOST_TEST_CHECKPOINT("IFE NO PENALTY");
  penalty = i_interp(&i2).penalty();
  BOOST_TEST(penalty == 0.0);
  penalty = i_interp(&i3).penalty();
  BOOST_TEST(penalty == 0.0);

  BOOST_TEST_CHECKPOINT("IFE SAME RESULT PENALTY");
  const i_mep i4({
                   {{f_ife, {1, 2, 2, 2}}},  // [0] FIFE [1], [2], [2], [2]
                   {{   c0,         null}},  // [1] 0.0
                   {{   c1,         null}}   // [2] 1.0
                 });
  penalty = i_interp(&i4).penalty();
  BOOST_TEST(penalty > 0.0);
}

BOOST_AUTO_TEST_CASE(IFZ, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("IFZ(0,1,0) == 1");
  const i_mep i1({
                   {{f_ifz, {1, 2, 1}}},  // [0] FIFZ [1], [2], [1]
                   {{   c0,      null}},  // [1] 0.0
                   {{   c1,      null}}   // [2] 1.0
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("IFE(1,1,0) == 0");
  const i_mep i2({
                   {{f_ifz, {2, 2, 1}}},  // [0] FIFZ [2], [2], [1]
                   {{   c0,      null}},  // [1] 0.0
                   {{   c1,      null}}   // [2] 1.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("IFZ(Z,Z,Z-Z) == 0");
  const i_mep i3({
                   {{f_ifz, {2, 2, 1}}},  // [0] FIFZ Z, Z, [1]
                   {{f_sub,    {2, 2}}},  // [1] FSUB Z, Z
                   {{    z,      null}}   // [2] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000000.0, 1000000.0);
    ret = i_interp(&i3).run();

    BOOST_TEST(real::base(ret) == 0.0);
  }
}

BOOST_AUTO_TEST_CASE(MAX, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("MAX(0,0) == 0");
  const i_mep i1({
                   {{f_max, {1, 2}}},  // [0] FMAX [1], [2]
                   {{   c0,   null}},  // [1] 0.0
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("MAX(0,1) == 1");
  const i_mep i2({
                   {{f_max, {1, 2}}},  // [0] FMAX [1], [2]
                   {{   c0,   null}},  // [1] 0.0
                   {{   c1,   null}}   // [2] 1.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("MAX(X,Y) == (X>Y ? X : Y)");
  const i_mep i3({
                   {{f_max, {1, 2}}},  // [0] FMAX [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = i_interp(&i3).run();
  const auto res1(real::base(ret));
  BOOST_TEST(res1 == std::max(real::base(x->eval(nullptr)),
                              real::base(y->eval(nullptr))));

  BOOST_TEST_CHECKPOINT("MAX(X,Y) == MAX(Y,X)");
  const i_mep i4({
                   {{f_max, {2, 1}}},  // [0] FMAX [2], [1]
                   {{    x,   null}},  // [1] X
                   {{    y,   null}}   // [2] Y
                 });
  ret = i_interp(&i4).run();
  BOOST_TEST(real::base(ret) == res1);
}

BOOST_AUTO_TEST_CASE(MUL, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("MUL(X,0) == 0");
  const i_mep i1({
                   {{f_mul, {1, 2}}},  // [0] FMUL [1], [2]
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("MUL(X,1) == X");
  const i_mep i2({
                   {{f_mul, {1, 2}}},  // [0] FMUL 1, 2
                   {{    x,   null}},  // [1] X
                   {{   c1,   null}}   // [2] 1.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("MUL(X,2) == ADD(X,X)");
  const i_mep i3({
                   {{f_sub, {1, 2}}},  // [0] FSUB 1, 2
                   {{f_add, {3, 3}}},  // [1] FADD 3, 3
                   {{f_mul, {3, 4}}},  // [2] FMUL 3, 4
                   {{    x,   null}},  // [3] X
                   {{   c2,   null}}   // [4] 2.0
                 });
  ret = i_interp(&i3).run();
  BOOST_TEST(real::base(ret) == 0.0);
}

BOOST_AUTO_TEST_CASE(SQRT, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("SQRT(1) == 1");
  const i_mep i1({
                   {{f_sqrt,  {1}}},  // [0] SQRT [1]
                   {{    c1, null}}   // [1] 1.0
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 1.0);

  BOOST_TEST_CHECKPOINT("SQRT(-X) == NaN");
  const i_mep i2({
                   {{f_sqrt,  {1}}},  // [0] FSQRT [1]
                   {{neg_x,  null}}   // [1] -X
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(ret.empty());

  BOOST_TEST_CHECKPOINT("SQRT(Z) = std::sqrt(Z)");
  const i_mep i3({
                   {{f_sqrt,  {1}}},  // [0] FSQRT [1]
                   {{     z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.0, 1000000.0);
    ret = i_interp(&i3).run();
    BOOST_TEST(real::base(ret) == std::sqrt(static_cast<Z *>(z)->val));
  }
}

BOOST_AUTO_TEST_CASE(SUB, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("SUB(X,-X) == 0");
  const i_mep i1({
                   {{f_sub, {1, 2}}},  // [0] FSUB [1], [2]
                   {{    x,   null}},  // [1] X
                   {{    x,   null}}   // [2] X
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("SUB(X,0) == X");
  const i_mep i2({
                   {{f_sub, {1, 2}}},  // [0] FSUB [1], [2]
                   {{    x,   null}},  // [1] X
                   {{   c0,   null}}   // [2] 0.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(real::base(ret) == real::base(x->eval(nullptr)));

  BOOST_TEST_CHECKPOINT("SUB(Z,X) == Z-X");
  const i_mep i3({
                   {{f_sub, {1, 2}}},  // [0] FSUB [1], [2]
                   {{    z,   null}},  // [1] Z
                   {{    x,   null}}   // [2] X
                 });
  for (unsigned j(0); j < 1000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000.0, 1000.0);
    ret = i_interp(&i3).run();

    const auto v1(real::base(ret));
    const auto v2(static_cast<Z *>(z)->val - real::base(x->eval(nullptr)));
    BOOST_TEST(v1 == v2);
  }
}

BOOST_AUTO_TEST_CASE(LN, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("LN(1) == 0");
  const i_mep i1({
                   {{f_ln,  {1}}},  // [0] FLN [1]
                   {{  c1, null}}   // [1] 1.0
                 });
  ret = i_interp(&i1).run();
  BOOST_TEST(real::base(ret) == 0.0);

  BOOST_TEST_CHECKPOINT("LN(0) == NaN");
  const i_mep i2({
                   {{f_ln,  {1}}},  // [0] FLN [1]
                   {{  c0, null}}   // [1] 0.0
                 });
  ret = i_interp(&i2).run();
  BOOST_TEST(ret.empty());

  BOOST_TEST_CHECKPOINT("LN(Z) = std::log(Z)");
  const i_mep i3({
                   {{f_ln,  {1}}},  // [0] LN 1
                   {{   z, null}}   // [1] Z
                 });
  for (unsigned j(0); j < 100; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.1, 1000000.0);
    ret = i_interp(&i3).run();
    BOOST_TEST(real::base(ret) == std::log(static_cast<Z *>(z)->val));
  }
}

BOOST_AUTO_TEST_SUITE_END()
