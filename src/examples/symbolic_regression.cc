/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2018 EOS di Manlio Morini.
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
  vita::src_problem prob;

  // DATA
  const auto function([](double x) { return x + std::sin(x); });
  const auto sample([&function](double x) { return std::to_string(function(x))
                                                   + ","
                                                   + std::to_string(x)
                                                   + "\n"; });
  std::istringstream training(
    sample(-10) + sample(-8) + sample(-6) + sample(-4) + sample(-2)
    + sample(0) + sample( 2) + sample( 4) + sample( 6) + sample( 8));

  prob.data().read_csv(training);

  // SYMBOLS
  prob.sset.insert<vita::real::sin>();
  prob.sset.insert<vita::real::cos>();
  prob.sset.insert<vita::real::add>();
  prob.sset.insert<vita::real::sub>();
  prob.sset.insert<vita::real::div>();
  prob.sset.insert<vita::real::mul>();
  prob.setup_terminals();

  // SEARCH & RESULT
  vita::src_search<> s(prob);
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::print_format(vita::out::c_language_f)
            << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
