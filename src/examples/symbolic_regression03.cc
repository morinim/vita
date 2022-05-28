/*
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/symbolic_regression_part2
 */

/* CHANGES IN THIS FILE MUST BE APPLIED TO THE LINKED WIKI PAGE */

#include "kernel/vita.h"

const double a = vita::random::between(-10.0, 10.0);
const double b = vita::random::between(-10.0, 10.0);

class c : public vita::terminal
{
public:
  c() : vita::terminal("c") {}

  vita::value_t eval(vita::symbol_params &) const override
  {
    static const double val(vita::random::between(-10.0, 10.0));
    return val;
  }
};

using candidate_solution = vita::i_mep;

// Given an individual (i.e. a candidate solution of the problem), returns an
// score measuring how good it is.
class my_evaluator : public vita::evaluator<candidate_solution>
{
public:
  vita::fitness_t operator()(const candidate_solution &x) override
  {
    const auto ret(vita::run(x));

    const double f(vita::has_value(ret) ? std::get<vita::D_DOUBLE>(ret)
                                        : 0.0);

    const double model_output(b * f);

    const double delta(std::fabs(a - model_output));

    return {-delta};
  }
};


int main()
{
  vita::problem prob;

  // SETTING UP SYMBOLS
  prob.insert<c>();
  prob.insert<vita::real::add>();
  prob.insert<vita::real::sub>();
  prob.insert<vita::real::mul>();

  // AD HOC EVALUATOR
  vita::search<candidate_solution> s(prob);
  s.training_evaluator<my_evaluator>();

  // SEARCHING
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
