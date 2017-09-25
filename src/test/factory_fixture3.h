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

#if !defined(VITA_FACTORY_FIXTURE3_H)
#define      VITA_FACTORY_FIXTURE3_H

#include "kernel/problem.h"
#include "kernel/interpreter.h"
#include "kernel/terminal.h"
#include "kernel/src/primitive/factory.h"

struct F_FACTORY3
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0) {}

    bool input() const override { return true; }

    vita::any eval(vita::core_interpreter *) const override
    { return vita::any(val); }

    double val;
  };

  F_FACTORY3() : prob(vita::initialization::standard), factory(), null()
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY3)");

    c0 = prob.sset.insert(factory.make("0.0"));
    c1 = prob.sset.insert(factory.make("1.0"));
    c2 = prob.sset.insert(factory.make("2.0"));
    c3 = prob.sset.insert(factory.make("3.0"));
    x = prob.sset.insert(factory.make("123.0"));
    neg_x = prob.sset.insert(factory.make("-123.0"));
    y = prob.sset.insert(factory.make("321.0"));
    z = prob.sset.insert(std::make_unique<Z>());
    f_abs = prob.sset.insert(factory.make("FABS"));
    f_add = prob.sset.insert(factory.make("FADD"));
    f_aq = prob.sset.insert(factory.make("FAQ"));
    f_div = prob.sset.insert(factory.make("FDIV"));
    f_idiv = prob.sset.insert(factory.make("FIDIV"));
    f_ife = prob.sset.insert(factory.make("FIFE"));
    f_ifz = prob.sset.insert(factory.make("FIFZ"));
    f_ln = prob.sset.insert(factory.make("FLN"));
    f_max = prob.sset.insert(factory.make("FMAX"));
    f_mul = prob.sset.insert(factory.make("FMUL"));
    f_sqrt = prob.sset.insert(factory.make("FSQRT"));
    f_sub = prob.sset.insert(factory.make("FSUB"));

    prob.env.code_length = 32;
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
  vita::symbol *f_aq;
  vita::symbol *f_div;
  vita::symbol *f_idiv;
  vita::symbol *f_ife;
  vita::symbol *f_ifz;
  vita::symbol *f_ln;
  vita::symbol *f_max;
  vita::symbol *f_mul;
  vita::symbol *f_sqrt;
  vita::symbol *f_sub;

  vita::problem           prob;
  vita::symbol_factory factory;

  vita::any ret;

  const std::vector<vita::index_t> null;
};

#endif  // include guard
