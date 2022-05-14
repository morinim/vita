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
 *  \see https://github.com/morinim/vita/wiki/symbolic_regression_part4
 */

/* CHANGES IN THIS FILE MUST BE APPLIED TO THE LINKED WIKI PAGE */

#include "kernel/vita.h"

constexpr std::size_t    N(6);
constexpr std::size_t VARS(3);

struct example
{
  example(const std::vector<double> &ex_a, const vita::matrix<double> &ex_b,
          const std::vector<double> &ex_x)
    : a(ex_a), b(ex_b), x()
  {
    std::copy(ex_x.begin(), ex_x.end(), std::back_inserter(x));
  }

  std::vector<double>        a;
  vita::matrix<double>       b;
  std::vector<vita::value_t> x;
};

using training_set = std::vector<example>;

training_set get_training_set()
{
  const auto get_vector([](std::size_t s)
  {
    std::vector<double> v(s);
    std::generate_n(v.begin(), s,
                    [] { return vita::random::between(-10.0, 10.0); });
    return v;
  });

  const auto get_matrix([]
  {
    vita::matrix<double> m(N, N);
    std::generate(m.begin(), m.end(),
                  [] { return vita::random::between(-10.0, 10.0); });
    return m;
  });

  training_set ret;

  std::generate_n(std::back_inserter(ret), 1000,
                  [get_vector, get_matrix]
                  {
                    return example(get_vector(N),
                                   get_matrix(),
                                   get_vector(VARS));
                  });

  return ret;
}

using candidate_solution = vita::team<vita::i_mep>;

class error_functor
{
public:
  error_functor(const candidate_solution &s) : s_(s) {}

  double operator()(const example &ex) const
  {
    std::vector<double> f(N);
    std::transform(s_.begin(), s_.end(), f.begin(),
                   [&ex](const auto &i)
                   {
                     const auto ret(vita::run(i, ex.x));

                     return vita::has_value(ret) ? std::get<vita::D_DOUBLE>(ret)
                                                 : 0.0;
                   });

    std::vector<double> model(N, 0.0);
    for (unsigned i(0); i < N; ++i)
      for (unsigned j(0); j < N; ++j)
        model[i] += ex.b(i, j) * f[j];

    double delta(std::inner_product(ex.a.begin(), ex.a.end(),
                                    model.begin(), 0.0,
                                    std::plus<>(),
                                    [](auto v1, auto v2)
                                    {
                                      return std::fabs(v1 - v2);
                                    }));

    return delta;
  }

private:
  candidate_solution s_;
};

// Given a team (i.e. a candidate solution of the problem), returns a score
// measuring how good it performs on a given dataset.
class my_evaluator
  : public vita::sum_of_errors_evaluator<candidate_solution, error_functor,
                                         training_set>
{
public:
  explicit my_evaluator(training_set &d)
    : sum_of_errors_evaluator<candidate_solution, error_functor,
                              training_set>(d)
  {}
};

int main()
{
  training_set data(get_training_set());

  vita::problem prob;
  prob.env.team.individuals = N;

  // SETTING UP SYMBOLS
  prob.sset.insert<vita::variable>("x1", 0);
  prob.sset.insert<vita::variable>("x2", 1);
  prob.sset.insert<vita::variable>("x3", 2);
  prob.insert<vita::real::add>();
  prob.insert<vita::real::sub>();
  prob.insert<vita::real::mul>();

  // AD HOC EVALUATOR
  vita::search<candidate_solution> s(prob);
  s.training_evaluator<my_evaluator>(data);

  // SEARCHING
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
