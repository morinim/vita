/**
 *
 *  \file factory_fixture4.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined FACTORY_FIXTURE4_H
#define      FACTORY_FIXTURE4_H

#include "kernel/environment.h"
#include "kernel/interpreter.h"
#include "kernel/src/primitive/factory.h"
#include "kernel/terminal.h"

struct F_FACTORY4
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0, true) {}

    vita::any eval(vita::interpreter *) const { return vita::any(val); }

    int val;
  };

  F_FACTORY4() : env(true), null({})
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY4)");

    static vita::symbol_factory &factory(vita::symbol_factory::instance());

    c0 = sset.insert(factory.make("0", {}));
    c1 = sset.insert(factory.make("1", {}));
    c2 = sset.insert(factory.make("2", {}));
    c3 = sset.insert(factory.make("3", {}));
    x = sset.insert(factory.make("123", {}));
    neg_x = sset.insert(factory.make("-123", {}));
    y = sset.insert(factory.make("321", {}));
    z = sset.insert(vita::make_unique<Z>());
    i_add = sset.insert(factory.make("ADD", {}));
    i_div = sset.insert(factory.make("DIV", {}));
    i_ife = sset.insert(factory.make("IFE", {}));
    i_mul = sset.insert(factory.make("MUL", {}));
    i_sub = sset.insert(factory.make("SUB", {}));

    env.code_length = 32;
  }

  ~F_FACTORY4()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY4)");
  }

  vita::symbol *c0;
  vita::symbol *c1;
  vita::symbol *c2;
  vita::symbol *c3;
  vita::symbol *x;
  vita::symbol *neg_x;
  vita::symbol *y;
  vita::symbol *z;

  vita::symbol *i_add;
  vita::symbol *i_div;
  vita::symbol *i_ife;
  vita::symbol *i_mul;
  vita::symbol *i_sub;

  vita::environment env;
  vita::symbol_set sset;
  vita::any ret;

  const std::vector<vita::index_t> null;
};

#endif  // FACTORY_FIXTURE4_H
