/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>

#include "kernel/vita.h"

int main()
{
  vita::src_problem titanic("../examples/titanic_train.csv");

  if (!titanic)
    return EXIT_FAILURE;

  vita::src_search<> s(titanic, vita::metric_flags::accuracy);
  const auto summary(s.run(10));                   // starting search (10 runs)
                                                   // and getting a summary

  std::cout << summary.best.solution << '\n'
            << summary.best.score.accuracy << '\n';
}
