/*
 *  \file
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
  std::istringstream training(  // the target function is `x + sin(x)`
    "-9.456,-10.0\n"
    "-8.989, -8.0\n"
    "-5.721, -6.0\n"
    "-3.243, -4.0\n"
    "-2.909, -2.0\n"
    " 0.000,  0.0\n"
    " 2.909,  2.0\n"
    " 3.243,  4.0\n"
    " 5.721,  6.0\n"
    " 8.989,  8.0\n");

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
  vita::src_search s(prob);
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
