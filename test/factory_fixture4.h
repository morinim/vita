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

#include "environment.h"
#include "interpreter.h"
#include "primitive/factory.h"
#include "terminal.h"

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

    c0 = factory.make("0", {});
    c1 = factory.make("1", {});
    c2 = factory.make("2", {});
    c3 = factory.make("3", {});
    x = factory.make("123", {});
    neg_x = factory.make("-123", {});
    y = factory.make("321", {});
    z = std::make_shared<Z>();
    i_add = factory.make("ADD", {});
    i_div = factory.make("DIV", {});
    i_ife = factory.make("IFE", {});
    i_mul = factory.make("MUL", {});
    i_sub = factory.make("SUB", {});

    env.insert(c0);
    env.insert(c1);
    env.insert(c2);
    env.insert(c3);
    env.insert(x);
    env.insert(neg_x);
    env.insert(y);
    env.insert(z);
    env.insert(i_add);
    env.insert(i_ife);
    env.insert(i_div);
    env.insert(i_mul);
    env.insert(i_sub);

    env.code_length = 32;
  }

  ~F_FACTORY4()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY4)");
  }

  vita::symbol::ptr c0;
  vita::symbol::ptr c1;
  vita::symbol::ptr c2;
  vita::symbol::ptr c3;
  vita::symbol::ptr x;
  vita::symbol::ptr neg_x;
  vita::symbol::ptr y;
  vita::symbol::ptr z;

  vita::symbol::ptr i_add;
  vita::symbol::ptr i_div;
  vita::symbol::ptr i_ife;
  vita::symbol::ptr i_mul;
  vita::symbol::ptr i_sub;

  vita::environment env;
  vita::any ret;

  const std::vector<vita::index_t> null;
};

#endif  // FACTORY_FIXTURE4_H
