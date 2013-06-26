/**
 *
 *  \file test_primitive_i.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>

#include "kernel/individual.h"
#include "kernel/random.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE primitive
#include "boost/test/unit_test.hpp"

using namespace boost;

#include "factory_fixture4.h"
#endif

BOOST_FIXTURE_TEST_SUITE(primitive_i, F_FACTORY4)

BOOST_AUTO_TEST_CASE(ADD)
{
  using namespace vita;
  vita::individual i(env, sset);

  BOOST_TEST_CHECKPOINT("ADD(X,0) == X");
  std::vector<gene> g(
  {
    {{i_add, {1, 2}}},  // [0] ADD 1,2
    {{   c0,   null}},  // [1] 0
    {{    x,   null}}   // [2] X
  });
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == any_cast<int>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == X+Y");
  g =
  {
    {{i_add, {1, 2}}},  // [0] ADD 1,2
    {{    y,   null}},  // [1] Y
    {{    x,   null}}   // [2] X
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == any_cast<int>(y->eval(0)) +
                        any_cast<int>(x->eval(0)), "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,-X) == 0");
  g =
  {
    {{i_add, {1, 2}}},  // [0] ADD 1,2
    {{    x,   null}},  // [1] X
    {{neg_x,   null}}   // [2] -X
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("ADD(X,Y) == ADD(Y,X)");
  g =
  {
    {{i_sub, {1, 2}}},  // [0] SUB 1,2
    {{i_add, {3, 4}}},  // [1] ADD 3,4
    {{i_add, {4, 3}}},  // [2] ADD 4,3
    {{    x,   null}},  // [3] X
    {{    y,   null}},  // [4] Y
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(DIV)
{
  using namespace vita;
  vita::individual i(env, sset);

  BOOST_TEST_CHECKPOINT("DIV(X,X) == 1");
  std::vector<gene> g(
  {
    {{i_div, {1, 2}}},  // [0] DIV 1, 2
    {{    x,   null}},  // [1] X
    {{    x,   null}}   // [2] X
  });
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(X,1) == X");
  g =
  {
    {{i_div, {1, 2}}},  // [0] DIV 1, 2
    {{    x,   null}},  // [1] X
    {{   c1,   null}}   // [2] 1
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == any_cast<int>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(-X,X) == -1");
  g =
  {
    {{i_div, {1, 2}}},  // [0] DIV 1, 2
    {{neg_x,   null}},  // [1] -X
    {{    x,   null}}   // [2] X
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == -1, "\n" << i);

  BOOST_TEST_CHECKPOINT("DIV(X,0) == X");
  g =
  {
    {{i_div, {1, 2}}},  // [0] DIV 1, 2
    {{    x,   null}},  // [1] X
    {{   c0,   null}}   // [2] 0
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == any_cast<int>(x->eval(0)),
                        "\n" << i);
}

BOOST_AUTO_TEST_CASE(IFE)
{
  using namespace vita;
  vita::individual i(env, sset);

  BOOST_TEST_CHECKPOINT("IFE(0,0,1,0) == 1");
  std::vector<gene> g(
  {
    {{i_ife, {1, 1, 2, 1}}},  // [0] IFE 1,1,2,1
    {{   c0,         null}},  // [1] 0
    {{   c1,         null}}   // [2] 1
  });
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 1, "\n" << i);

  BOOST_TEST_CHECKPOINT("IFE(0,1,1,0) == 0");
  g =
  {
    {{i_ife, {1, 2, 2, 1}}},  // [0] IFE 1,2,2,1
    {{   c0,         null}},  // [1] 0
    {{   c1,         null}}   // [2] 1
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("IFE(Z,X,1,0) == 0");
  g =
  {
    {{i_ife, {1, 2, 3, 4}}},  // [0] IFE Z, X, 1, 0
    {{    z,         null}},  // [1] Z
    {{    x,         null}},  // [2] X
    {{   c1,         null}},  // [2] 1
    {{   c0,         null}}   // [1] 0
  };
  static_cast<Z *>(z)->val = 0;
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(MUL)
{
  using namespace vita;
  vita::individual i(env, sset);

  BOOST_TEST_CHECKPOINT("MUL(X,0) == 0");
  std::vector<gene> g(
  {
    {{i_mul, {1, 2}}},  // [0] MUL 1, 2
    {{    x,   null}},  // [1] X
    {{   c0,   null}}   // [2] 0
  });
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("MUL(X,1) == X");
  g =
  {
    {{i_mul, {1, 2}}},  // [0] MUL 1, 2
    {{    x,   null}},  // [1] X
    {{   c1,   null}}   // [2] 1
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == any_cast<int>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("MUL(X,2) == ADD(X,X)");
  g =
  {
    {{i_sub, {1, 2}}},  // [0] SUB 1, 2
    {{i_add, {3, 3}}},  // [1] ADD 3, 3
    {{i_mul, {3, 4}}},  // [2] MUL 3, 4
    {{    x,   null}},  // [3] X
    {{   c2,   null}}   // [4] 2
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);
}

BOOST_AUTO_TEST_CASE(SUB)
{
  using namespace vita;
  vita::individual i(env, sset);

  BOOST_TEST_CHECKPOINT("SUB(X,-X) == 0");
  std::vector<gene> g(
  {
    {{i_sub, {1, 2}}},  // [0] SUB 1, 2
    {{    x,   null}},  // [1] X
    {{    x,   null}}   // [2] X
  });
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == 0, "\n" << i);

  BOOST_TEST_CHECKPOINT("SUB(X,0) == X");
  g =
  {
    {{i_sub, {1, 2}}},  // [0] SUB 1, 2
    {{    x,   null}},  // [1] X
    {{   c0,   null}}   // [2] 0
  };
  ret = interpreter(i.replace(g)).run();
  BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) == any_cast<int>(x->eval(0)),
                        "\n" << i);

  BOOST_TEST_CHECKPOINT("SUB(Z,X) == Z-X");
  g =
  {
    {{i_sub, {1, 2}}},  // [0] SUB 1, 2
    {{    z,   null}},  // [1] Z
    {{    x,   null}}   // [2] X
  };
  for (unsigned j(0); j < 1000; ++j)
  {
    static_cast<Z *>(z)->val = vita::random::between<int>(-1000, 1000);
    ret = interpreter(i.replace(g)).run();
    BOOST_REQUIRE_MESSAGE(any_cast<int>(ret) ==
                          static_cast<Z *>(z)->val -
                          any_cast<int>(x->eval(0)), "\n" << i);
  }
}

BOOST_AUTO_TEST_SUITE_END()
