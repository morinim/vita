/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FIXTURE4_H)
#define      FIXTURE4_H

#include "kernel/gp/src/primitive/factory.h"
#include "kernel/gp/src/primitive/int.h"
#include "kernel/gp/terminal.h"
#include "kernel/problem.h"

struct fixture4
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0) {}

    bool input() const override { return true; }

    vita::value_t eval(vita::symbol_params &) const override { return val; }

    int val;
  };

  static constexpr vita::integer::base_t x_val = 123;
  static constexpr vita::integer::base_t y_val = 321;

  fixture4() : prob(), factory(), null({})
  {
    prob.env.init();

    c0 = prob.sset.insert(factory.make("0"));
    c1 = prob.sset.insert(factory.make("1"));
    c2 = prob.sset.insert(factory.make("2"));
    c3 = prob.sset.insert(factory.make("3"));
    x = prob.sset.insert(factory.make(std::to_string(x_val)));
    neg_x = prob.sset.insert(factory.make(std::to_string(-x_val)));
    y = prob.sset.insert(factory.make(std::to_string(y_val)));
    z = prob.sset.insert<Z>();
    i_add = prob.sset.insert(factory.make("ADD"));
    i_div = prob.sset.insert(factory.make("DIV"));
    i_ife = prob.sset.insert(factory.make("IFE"));
    i_mul = prob.sset.insert(factory.make("MUL"));
    i_sub = prob.sset.insert(factory.make("SUB"));

    prob.env.mep.code_length = 32;
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

  vita::value_t ret;

  const std::vector<vita::index_t> null;
};

#endif  // include guard
