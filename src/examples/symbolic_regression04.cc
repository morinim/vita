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
 *  \see https://github.com/morinim/vita/wiki/symbolic_regression_part3
 */

/* CHANGES IN THIS FILE MUST BE APPLIED TO THE LINKED WIKI PAGE */

#include "kernel/vita.h"

constexpr std::size_t N(6);

std::vector<double> get_vector()
{
  std::vector<double> ret(N);

  std::generate_n(ret.begin(), N,
                  [] { return vita::random::between(-10.0, 10.0); });

  return ret;
}

vita::matrix<double> get_matrix()
{
  vita::matrix<double> ret(N, N);

  std::generate(ret.begin(), ret.end(),
                [] { return vita::random::between(-10.0, 10.0); });

  return ret;
}

const auto a = get_vector();  // N-dimensional vector
const auto b = get_matrix();  // NxN matrix

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

using candidate_solution = vita::team<vita::i_mep>;

// Given a team (i.e. a candidate solution of the problem), returns a score
// measuring how good it is.
class my_evaluator : public vita::evaluator<candidate_solution>
{
public:
  vita::fitness_t operator()(const candidate_solution &x) override
  {
    std::vector<double> f(N);
    std::transform(x.begin(), x.end(), f.begin(),
                   [](const auto &prg)
                   {
                     const auto ret(vita::run(prg));

                     return vita::has_value(ret) ? std::get<vita::D_DOUBLE>(ret)
                                                 : 0.0;
                   });

    std::vector<double> model(N, 0.0);
    for (unsigned i(0); i < N; ++i)
      for (unsigned j(0); j < N; ++j)
        model[i] += b(i, j) * f[j];

    double delta(std::inner_product(a.begin(), a.end(), model.begin(), 0.0,
                                    std::plus<>(),
                                    [](auto v1, auto v2)
                                    {
                                      return std::fabs(v1 - v2);
                                    }));

    return {-delta};
  }
};


int main()
{
  vita::problem prob;

  prob.env.team.individuals = N;

  // SETTING UP SYMBOLS
  prob.sset.insert<c>();
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
