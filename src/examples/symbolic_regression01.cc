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
 *  \see https://github.com/morinim/vita/wiki/symbolic_regression01
 */

#include "kernel/vita.h"

int main()
{
  // DATA SAMPLE
  // (the target function is `ln(x*x + y*y)`)
  std::istringstream training(R"(
    -2.079, 0.25, 0.25
    -0.693, 0.50, 0.50
     0.693, 1.00, 1.00
     0.000, 0.00, 1.00
     0.000, 1.00, 0.00
     1.609, 1.00, 2.00
     1.609, 2.00, 1.00
     2.079, 2.00, 2.00
  )");

  // READING INPUT DATA
  vita::src_problem prob(training);

  // SETTING UP SYMBOLS
  prob.insert<vita::real::sin>();
  prob.insert<vita::real::add>();
  prob.insert<vita::real::sub>();
  prob.insert<vita::real::mul>();
  prob.insert<vita::real::ln>();

  // SEARCHING
  vita::src_search s(prob);
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
