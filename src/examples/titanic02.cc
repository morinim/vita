/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2022 EOS di Manlio Morini.
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

  src_search s(titanic, metric_flags::accuracy);  // starting search...
  const auto summary(s.run());                    // ...and getting a summary

  std::cout << summary.best.solution << '\n'
            << summary.best.score.accuracy << '\n';
}
