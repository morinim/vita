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

#if !defined(FIXTURE3_H)
#define      FIXTURE3_H

#include "kernel/gp/src/primitive/factory.h"
#include "kernel/gp/src/primitive/real.h"
#include "kernel/gp/terminal.h"
#include "kernel/problem.h"

struct fixture3
{
  class Z : public vita::terminal
  {
  public:
    Z() : vita::terminal("Z", 0) {}

    bool input() const override { return true; }

    vita::value_t eval(vita::symbol_params &) const override { return val; }

    double val;
  };

  static constexpr vita::real::base_t x_val = 123.0;
  static constexpr vita::real::base_t y_val = 321.0;

  fixture3() : prob(), factory(), null(),
               c0(prob.sset.insert(factory.make("0.0"))),
               c1(prob.sset.insert(factory.make("1.0"))),
               c2(prob.sset.insert(factory.make("2.0"))),
               c3(prob.sset.insert(factory.make("3.0"))),
               x(prob.sset.insert(factory.make(std::to_string(x_val)))),
               neg_x(prob.sset.insert(factory.make(std::to_string(-x_val)))),
               y(prob.sset.insert(factory.make(std::to_string(y_val)))),
               z(prob.sset.insert<Z>()),
               f_abs(prob.sset.insert(factory.make("FABS"))),
               f_add(prob.sset.insert(factory.make("FADD"))),
               f_aq(prob.sset.insert(factory.make("FAQ"))),
               f_cos(prob.sset.insert(factory.make("FCOS"))),
               f_div(prob.sset.insert(factory.make("FDIV"))),
               f_idiv(prob.sset.insert(factory.make("FIDIV"))),
               f_ife(prob.sset.insert(factory.make("FIFE"))),
               f_ifz(prob.sset.insert(factory.make("FIFZ"))),
               f_ln(prob.sset.insert(factory.make("FLN"))),
               f_max(prob.sset.insert(factory.make("FMAX"))),
               f_mul(prob.sset.insert(factory.make("FMUL"))),
               f_sigmoid(prob.sset.insert(factory.make("FSIGMOID"))),
               f_sin(prob.sset.insert(factory.make("FSIN"))),
               f_sqrt(prob.sset.insert(factory.make("FSQRT"))),
               f_sub(prob.sset.insert(factory.make("FSUB")))
  {
    prob.env.init().mep.code_length = 32;
  }

  vita::problem           prob;
  vita::symbol_factory factory;

  vita::value_t ret;

  const std::vector<vita::index_t> null;

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
  vita::symbol *f_cos;
  vita::symbol *f_div;
  vita::symbol *f_idiv;
  vita::symbol *f_ife;
  vita::symbol *f_ifz;
  vita::symbol *f_ln;
  vita::symbol *f_max;
  vita::symbol *f_mul;
  vita::symbol *f_sigmoid;
  vita::symbol *f_sin;
  vita::symbol *f_sqrt;
  vita::symbol *f_sub;
};

#endif  // include guard
