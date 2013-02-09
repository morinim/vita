/**
 *
 *  \file factory_fixture3.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined FACTORY_FIXTURE3_H
#define      FACTORY_FIXTURE3_H

#include "environment.h"
#include "interpreter.h"
#include "primitive/factory.h"
#include "terminal.h"

struct F_FACTORY3
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0, true) {}

    vita::any eval(vita::interpreter *) const { return vita::any(val); }

    double val;
  };

  F_FACTORY3() : env(true), null({})
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY3)");

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
    f_ife = factory.make("FIFE", {});
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

  ~F_FACTORY3()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY3)");
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
  vita::symbol_ptr f_ife;
  vita::symbol_ptr f_ln;
  vita::symbol_ptr f_mul;
  vita::symbol_ptr f_sub;

  vita::environment env;
  vita::any ret;

  const std::vector<vita::index_t> null;
};

#endif  // FACTORY_FIXTURE3_H
