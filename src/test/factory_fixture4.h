/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined FACTORY_FIXTURE4_H
#define      FACTORY_FIXTURE4_H

#include "kernel/interpreter.h"
#include "kernel/problem.h"
#include "kernel/terminal.h"
#include "kernel/src/primitive/factory.h"

struct F_FACTORY4
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0) {}

    bool input() const override { return true; }

    vita::any eval(vita::core_interpreter *) const override
    { return vita::any(val); }

    int val;
  };

  F_FACTORY4() : prob(vita::initialization::standard), factory(), null({})
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY4)");

    c0 = prob.sset.insert(factory.make("0"));
    c1 = prob.sset.insert(factory.make("1"));
    c2 = prob.sset.insert(factory.make("2"));
    c3 = prob.sset.insert(factory.make("3"));
    x = prob.sset.insert(factory.make("123"));
    neg_x = prob.sset.insert(factory.make("-123"));
    y = prob.sset.insert(factory.make("321"));
    z = prob.sset.insert<Z>();
    i_add = prob.sset.insert(factory.make("ADD"));
    i_div = prob.sset.insert(factory.make("DIV"));
    i_ife = prob.sset.insert(factory.make("IFE"));
    i_mul = prob.sset.insert(factory.make("MUL"));
    i_sub = prob.sset.insert(factory.make("SUB"));

    prob.env.mep.code_length = 32;
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

  vita::problem           prob;
  vita::symbol_factory factory;

  vita::any ret;

  const std::vector<vita::index_t> null;
};

#endif  // include guard
