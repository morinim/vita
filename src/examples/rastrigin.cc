/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2019-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/rastrigin_tutorial
 */

#include "kernel/vita.h"

double neg_rastrigin(const std::vector<double> &x)
{
  constexpr double  A = 10.0;
  constexpr double PI =  3.141592653589793;

  const double rastrigin =
    A * x.size() + std::accumulate(x.begin(), x.end(), 0.0,
                                   [=](double sum, double xi)
                                   {
                                     return sum + xi*xi - A*std::cos(2*PI*xi);
                                   });

  return -rastrigin;
}

int main()
{
  const unsigned dimensions(5);  // 5D - Rastrigin function

  vita::de_problem prob(dimensions, {-5.12, 5.12});

  prob.env.individuals =   50;
  prob.env.generations = 1000;

  vita::de_search<decltype(neg_rastrigin)> search(prob, neg_rastrigin);

  const auto res(search.run());

  const auto solution(  res.best.solution);
  const auto value(res.best.score.fitness);

  std::cout << "Minimum found: " <<  value << '\n';

  std::cout << "Coordinates: [ ";
  for (auto xi : solution)
    std::cout << xi << ' ';
  std::cout << "]\n";
}
