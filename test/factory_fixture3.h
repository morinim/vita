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

#include "kernel/environment.h"
#include "kernel/interpreter.h"
#include "kernel/symbol_set.h"
#include "kernel/terminal.h"
#include "kernel/src/primitive/factory.h"

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

    c0 = sset.insert(factory.make("0.0", {}));
    c1 = sset.insert(factory.make("1.0", {}));
    c2 = sset.insert(factory.make("2.0", {}));
    c3 = sset.insert(factory.make("3.0", {}));
    x = sset.insert(factory.make("123.0", {}));
    neg_x = sset.insert(factory.make("-123.0", {}));
    y = sset.insert(factory.make("321.0", {}));
    z = sset.insert(vita::make_unique<Z>());
    f_abs = sset.insert(factory.make("FABS", {}));
    f_add = sset.insert(factory.make("FADD", {}));
    f_div = sset.insert(factory.make("FDIV", {}));
    f_idiv = sset.insert(factory.make("FIDIV", {}));
    f_ife = sset.insert(factory.make("FIFE", {}));
    f_ln = sset.insert(factory.make("FLN", {}));
    f_max = sset.insert(factory.make("FMAX", {}));
    f_mul = sset.insert(factory.make("FMUL", {}));
    f_sqrt = sset.insert(factory.make("FSQRT", {}));
    f_sub = sset.insert(factory.make("FSUB", {}));

    env.code_length = 32;
  }

  ~F_FACTORY3()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY3)");
  }

  vita::symbol *c0;
  vita::symbol *c1;
  vita::symbol *c2;
  vita::symbol *c3;
  vita::symbol *x;
  vita::symbol *neg_x;
  vita::symbol *y;
  vita::symbol *z;

  vita::symbol *f_abs;
  vita::symbol *f_add;
  vita::symbol *f_div;
  vita::symbol *f_idiv;
  vita::symbol *f_ife;
  vita::symbol *f_ln;
  vita::symbol *f_max;
  vita::symbol *f_mul;
  vita::symbol *f_sqrt;
  vita::symbol *f_sub;

  vita::environment env;
  vita::symbol_set sset;
  vita::any ret;

  const std::vector<vita::index_t> null;
};

#endif  // FACTORY_FIXTURE3_H
