/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
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

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE primitive
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr double epsilon(0.00001);

#include "factory_fixture3.h"
#endif

BOOST_FIXTURE_TEST_SUITE(primitive_d, F_FACTORY3)

BOOST_AUTO_TEST_CASE(ABS)
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("ABS(-X) == X");
  const i_mep i1(env, sset,
                 {
                   {{f_abs,  {1}}},  // [0] ABS 1
                   {{neg_x, null}}   // [1] -X
                 });
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("ABS(X) == X");
  const i_mep i2(env, sset,
                 {
                   {{f_abs,  {1}}},  // [0] ABS 1
                   {{    x, null}}   // [1] X
                 });
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);
}

BOOST_AUTO_TEST_CASE(ADD)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("ADD(X,0) == X");
  std::vector<gene> g(
  {
    {{f_add, {1, 2}}},  // [0] ADD 1,2
    {{   c0,   null}},  // [1] 0
    {{    x,   null}}   // [2] X
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == X+Y");
  g =
  {
    {{f_add, {1, 2}}},  // [0] ADD 1,2
    {{    y,   null}},  // [1] Y
    {{    x,   null}}   // [2] X
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(y->eval(nullptr)) +
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("ADD(X,-X) == 0");
  g =
  {
    {{f_add, {1, 2}}},  // [0] ADD 1,2
    {{    x,   null}},  // [1] X
    {{neg_x,   null}}   // [2] -X
  };
  const i_mep i3(env, sset, g);
  ret = i_interp(&i3).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == ADD(Y,X)");
  g =
  {
    {{f_sub, {1, 2}}},  // [0] SUB 1,2
    {{f_add, {3, 4}}},  // [1] ADD 3,4
    {{f_add, {4, 3}}},  // [2] ADD 4,3
    {{    x,   null}},  // [3] X
    {{    y,   null}}   // [4] Y
  };
  const i_mep i4(env, sset, g);
  ret = i_interp(&i4).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);
}

BOOST_AUTO_TEST_CASE(DIV)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("DIV(X,X) == 1");
  std::vector<gene> g(
  {
    {{f_div, {1, 2}}},  // [0] DIV 1, 2
    {{    x,   null}},  // [1] X
    {{    x,   null}}   // [2] X
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), 1.0, epsilon);

  BOOST_TEST_CHECKPOINT("DIV(X,1) == X");
  g =
  {
    {{f_div, {1, 2}}},  // [0] DIV 1, 2
    {{    x,   null}},  // [1] X
    {{   c1,   null}}   // [2] 1
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("DIV(-X,X) == -1");
  g =
  {
    {{f_div, {1, 2}}},  // [0] DIV 1, 2
    {{neg_x,   null}},  // [1] -X
    {{    x,   null}}   // [2] X
  };
  const i_mep i3(env, sset, g);
  ret = i_interp(&i3).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), -1.0, epsilon);

  BOOST_TEST_CHECKPOINT("DIV(X,0) == nan");
  g =
  {
    {{f_div, {1, 2}}},  // [0] DIV 1, 2
    {{    x,   null}},  // [1] X
    {{   c0,   null}}   // [2] 0
  };
  const i_mep i4(env, sset, g);
  ret = i_interp(&i4).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i4);
}

BOOST_AUTO_TEST_CASE(IDIV)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("IDIV(X,X) == 1");
  std::vector<gene> g(
  {
    {{f_idiv, {1, 2}}},  // [0] DIV 1, 2
    {{     x,   null}},  // [1] X
    {{     x,   null}}   // [2] X
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), 1.0, epsilon);

  BOOST_TEST_CHECKPOINT("IDIV(X,1) == X");
  g =
  {
    {{f_idiv, {1, 2}}},  // [0] IDIV 1, 2
    {{     x,   null}},  // [1] X
    {{    c1,   null}}   // [2] 1
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("IDIV(-X,X) == -1");
  g =
  {
    {{f_idiv, {1, 2}}},  // [0] IDIV 1, 2
    {{ neg_x,   null}},  // [1] -X
    {{     x,   null}}   // [2] X
  };
  const i_mep i3(env, sset, g);
  ret = i_interp(&i3).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), -1.0, epsilon);

  BOOST_TEST_CHECKPOINT("IDIV(3,2) == 1");
  g =
  {
    {{f_idiv, {1, 2}}},  // [0] IDIV 1, 2
    {{    c3,   null}},  // [1] 3
    {{    c2,   null}}   // [2] 2
  };
  const i_mep i4(env, sset, g);
  ret = i_interp(&i4).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), 1.0, epsilon);

  BOOST_TEST_CHECKPOINT("IDIV(X,0) == nan");
  g =
  {
    {{f_idiv, {1, 2}}},  // [0] IDIV 1, 2
    {{     x,   null}},  // [1] X
    {{    c0,   null}}   // [2] 0
  };
  const i_mep i5(env, sset, g);
  ret = i_interp(&i5).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i5);
}

BOOST_AUTO_TEST_CASE(IFE)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("IFE(0,0,1,0) == 1");
  std::vector<gene> g(
  {
    {{f_ife, {1, 1, 2, 1}}},  // [0] IFE 1,1,2,1
    {{   c0,         null}},  // [1] 0
    {{   c1,         null}}   // [2] 1
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), 1.0, epsilon);

  BOOST_TEST_CHECKPOINT("IFE(0,1,1,0) == 0");
  g =
  {
    {{f_ife, {1, 2, 2, 1}}},  // [0] IFE 1,2,2,1
    {{   c0,         null}},  // [1] 0
    {{   c1,         null}}   // [2] 1
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);

  BOOST_TEST_CHECKPOINT("IFE(Z,X,1,0) == 0");
  g =
  {
    {{f_ife, {1, 2, 3, 4}}},  // [0] IFE Z, X, 1, 0
    {{    z,         null}},  // [1] Z
    {{    x,         null}},  // [2] X
    {{   c1,         null}},  // [2] 1
    {{   c0,         null}}   // [1] 0
  };
  const i_mep i3(env, sset, g);
  static_cast<Z *>(z)->val = 0;
  ret = i_interp(&i3).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);
}

BOOST_AUTO_TEST_CASE(MAX)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("MAX(0,0) == 0");
  std::vector<gene> g(
  {
    {{f_max, {1, 2}}},  // [0] MAX 1, 2
    {{   c0,   null}},  // [1] 0
    {{   c0,   null}}   // [2] 0
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);

  BOOST_TEST_CHECKPOINT("MAX(0,1) == 1");
  g =
  {
    {{f_max, {1, 2}}},  // [0] MAX 1, 2
    {{   c0,   null}},  // [1] 0
    {{   c1,   null}}   // [2] 1
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), 1.0, epsilon);
}

BOOST_AUTO_TEST_CASE(MUL)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("MUL(X,0) == 0");
  std::vector<gene> g(
  {
    {{f_mul, {1, 2}}},  // [0] MUL 1, 2
    {{    x,   null}},  // [1] X
    {{   c0,   null}}   // [2] 0
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);

  BOOST_TEST_CHECKPOINT("MUL(X,1) == X");
  g =
  {
    {{f_mul, {1, 2}}},  // [0] MUL 1, 2
    {{    x,   null}},  // [1] X
    {{   c1,   null}}   // [2] 1
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("MUL(X,2) == ADD(X,X)");
  g =
  {
    {{f_sub, {1, 2}}},  // [0] SUB 1, 2
    {{f_add, {3, 3}}},  // [1] ADD 3, 3
    {{f_mul, {3, 4}}},  // [2] MUL 3, 4
    {{    x,   null}},  // [3] X
    {{   c2,   null}}   // [4] 2
  };
  const i_mep i3(env, sset, g);
  ret = i_interp(&i3).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);
}

BOOST_AUTO_TEST_CASE(SQRT)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("SQRT(1) == 1");
  std::vector<gene> g(
  {
    {{f_sqrt,  {1}}},  // [0] SQRT 1
    {{    c1, null}}   // [1] 1
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret), 1.0, epsilon);

  BOOST_TEST_CHECKPOINT("SQRT(-X) == nan");
  g =
  {
    {{f_sqrt,  {1}}},  // [0] SQRT 1
    {{neg_x,  null}}   // [1] -X
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i2);

  BOOST_TEST_CHECKPOINT("SQRT(Z) = std::sqrt(Z)");
  g =
  {
    {{f_sqrt,  {1}}},  // [0] LN 1
    {{     z, null}}   // [1] Z
  };
  const i_mep i3(env, sset, g);
  for (unsigned j(0); j < 1000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.0, 1000000.0);
    ret = i_interp(&i3).run();
    BOOST_CHECK_CLOSE(any_cast<double>(ret),
                      std::sqrt(static_cast<Z *>(z)->val), epsilon);
  }
}

BOOST_AUTO_TEST_CASE(SUB)
{
  using namespace vita;
  using i_interp = interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("SUB(X,-X) == 0");
  std::vector<gene> g(
  {
    {{f_sub, {1, 2}}},  // [0] SUB 1, 2
    {{    x,   null}},  // [1] X
    {{    x,   null}}   // [2] X
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);

  BOOST_TEST_CHECKPOINT("SUB(X,0) == X");
  g =
  {
    {{f_sub, {1, 2}}},  // [0] SUB 1, 2
    {{    x,   null}},  // [1] X
    {{   c0,   null}}   // [2] 0
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                      any_cast<double>(x->eval(nullptr)), epsilon);

  BOOST_TEST_CHECKPOINT("SUB(Z,X) == Z-X");
  g =
  {
    {{f_sub, {1, 2}}},  // [0] SUB 1, 2
    {{    z,   null}},  // [1] Z
    {{    x,   null}}   // [2] X
  };
  const i_mep i3(env, sset, g);

  for (unsigned j(0); j < 10000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(-1000.0, 1000.0);
    ret = i_interp(&i3).run();

    const auto v1(any_cast<double>(ret));
    const auto v2(static_cast<Z *>(z)->val -
                  any_cast<double>(x->eval(nullptr)));
    BOOST_CHECK_CLOSE(v1, v2, epsilon);
  }
}

BOOST_AUTO_TEST_CASE(LN)
{
  using namespace vita;
  using i_interp = vita::interpreter<i_mep>;

  BOOST_TEST_CHECKPOINT("LN(1) == 0");
  std::vector<gene> g(
  {
    {{f_ln,  {1}}},  // [0] LN 1
    {{  c1, null}}   // [1] 1
  });
  const i_mep i1(env, sset, g);
  ret = i_interp(&i1).run();
  BOOST_REQUIRE_SMALL(any_cast<double>(ret), epsilon);

  BOOST_TEST_CHECKPOINT("LN(0) == nan");
  g =
  {
    {{f_ln,  {1}}},  // [0] LN 1
    {{  c0, null}}   // [1] 0
  };
  const i_mep i2(env, sset, g);
  ret = i_interp(&i2).run();
  BOOST_REQUIRE_MESSAGE(ret.empty(), "\n" << i2);

  BOOST_TEST_CHECKPOINT("LN(Z) = std::log(Z)");
  g =
  {
    {{f_ln,  {1}}},  // [0] LN 1
    {{   z, null}}   // [1] Z
  };
  const i_mep i3(env, sset, g);
  for (unsigned j(0); j < 1000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between(0.1, 1000000.0);
    ret = i_interp(&i3).run();
    BOOST_REQUIRE_CLOSE(any_cast<double>(ret),
                        std::log(static_cast<Z *>(z)->val), epsilon);
  }
}

BOOST_AUTO_TEST_SUITE_END()
