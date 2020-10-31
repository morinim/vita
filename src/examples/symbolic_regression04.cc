/*
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/symbolic_regression
 */

#include "kernel/vita.h"

constexpr std::size_t DIM(6);

vita::matrix<double> get_input()
{
  vita::matrix<double> ret(DIM, DIM);

  std::generate(ret.begin(), ret.end(),
                [] { return vita::random::between(-10.0, 10.0); });

  return ret;
}

std::vector<double> get_phi()
{
  std::vector<double> ret(DIM);

  std::generate_n(ret.begin(), DIM,
                  [] { return vita::random::between(-10.0, 10.0); });

  return ret;
}

class V : public vita::terminal
{
public:
  V() : vita::terminal("V") {}

  vita::value_t eval(vita::core_interpreter *) const override
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
    static const auto input(get_input());
    static const auto phi(get_phi());

    std::vector<double> f(DIM);
    std::generate(
      f.begin(), f.end(),
      [&x, n = 0]() mutable
      {
        auto &f_n(x[n++]);
        const auto ret(vita::run(f_n));

        return vita::has_value(ret) ? std::get<vita::D_DOUBLE>(ret)
                                    : 0.0;
      });

    std::vector<double> model(DIM, 0.0);
    for (unsigned i(0); i < DIM; ++i)
      for (unsigned j(0); j < DIM; ++j)
        model[i] += input(i, j) * f[j];

    double delta(std::inner_product(phi.begin(), phi.end(), model.begin(), 0.0,
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

  prob.env.team.individuals = DIM;

  // SETTING UP SYMBOLS
  prob.sset.insert<V>();
  prob.insert<vita::real::add>();
  prob.insert<vita::real::sub>();
  prob.insert<vita::real::mul>();

  // AD HOC EVALUATOR
  vita::search<candidate_solution, vita::std_es> s(prob);
  s.training_evaluator<my_evaluator>();

  // SEARCHING
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
