/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2018-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/symbolic_regression
 */

#include "kernel/vita.h"

int main()
{
  // DATA SAMPLE
  // (the target function is `x + sin(x)`)
  std::istringstream training(R"(
    -9.456,-10.0
    -8.989, -8.0
    -5.721, -6.0
    -3.243, -4.0
    -2.909, -2.0
     0.000,  0.0
     2.909,  2.0
     3.243,  4.0
     5.721,  6.0
     8.989,  8.0
  )");

  // READING INPUT DATA
  vita::src_problem prob(training);

  // SETTING UP SYMBOLS
  prob.insert<vita::real::sin>();
  prob.insert<vita::real::cos>();
  prob.insert<vita::real::add>();
  prob.insert<vita::real::sub>();
  prob.insert<vita::real::div>();
  prob.insert<vita::real::mul>();

  // SEARCHING
  prob.env.generations = 50;
  vita::src_search s(prob);

  // This is a callback function invoked at the end of every generation and
  // useful to gather statistical data.
  s.after_generation([](const auto &pop, const auto &)
                     {
                       for (const auto &i : pop)
                         std::cout << vita::out::python_language << i << '\n';
                       std::cout << "--------------------------------------\n";
                       //std::cout << vita::out::python_language
                       //          << s.best.solution << '\n';
                     });

  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
