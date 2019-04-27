/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2018-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/nonogram_tutorial
 */

#include "kernel/vita.h"

int main()
{
  // TARGET FUNCTION
  const auto function = [](double x) { return x + std::sin(x); };

  // DATA SAMPLE
  const auto sample = [&function](double x) { return std::to_string(function(x))
                                                     + ","
                                                     + std::to_string(x)
                                                     + "\n"; };
  std::istringstream training(
    sample(-10) + sample(-8) + sample(-6) + sample(-4) + sample(-2)
    + sample(0) + sample( 2) + sample( 4) + sample( 6) + sample( 8));


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
  vita::src_search<> s(prob);
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
