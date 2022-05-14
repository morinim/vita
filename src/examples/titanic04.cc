/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2019-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/titanic_tutorial
 */

/* CHANGES IN THIS FILE MUST BE APPLIED TO THE LINKED WIKI PAGE */

#include <cstdlib>

#include "kernel/vita.h"

int main()
{
  using namespace vita;

  src_problem titanic("titanic_train.csv", src_problem::default_symbols);

  src_search s(titanic, metric_flags::accuracy);
  const auto summary(s.run());

  // Now, hopefully, we have a good classifier (`summary.best.solution`).
  // How can we exploit it?

  const auto model(s.lambdify(summary.best.solution));
  const auto example(random::element(titanic.data()));
  const auto result(model->tag(example));

  std::cout << "Correct class: " << label(example)
            << "   Prediction: " << result.label
            << "   Sureness: " << result.sureness << '\n';

  // We can make the model persistent...
  std::stringstream ss;
  serialize::save(ss, model);

  // ... and reload it when needed.
  const auto model2(serialize::lambda::load(ss, titanic.sset));
  const auto result2(model2->tag(example));
  std::cout << "   Prediction: " << result2.label
            << "   Sureness: " << result2.sureness << '\n';
  assert(result2.label == result.label);

  // If need be, individuals can be printed/exported in alternative languages:
  std::cout << "\nC LANGUAGE\n" << std::string(40, '-') << '\n'
            << out::c_language << summary.best.solution
            << "\n\nPYTHON LANGUAGE\n" << std::string(40, '-') << '\n'
            << out::python_language << summary.best.solution
            << "\n\nGRAPHVIZ FORMAT\n" << std::string(40, '-') << '\n'
            << out::graphviz << summary.best.solution
            << "\n\nLIST (DEBUG) FORMAT\n" << std::string(40, '-') << '\n'
            << out::list << summary.best.solution
            << '\n';
}
