/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>

#include "kernel/src/search.h"

int main()
{
  vita::src_problem titanic("titanic_train.csv");  // reading training set

  if (!titanic)
    return EXIT_FAILURE;

  vita::src_search<> s(titanic, vita::metric::accuracy);
  const auto summary(s.run(10));                   // starting search (10 runs)
                                                   // and getting a summary

  std::cout << summary.best.solution << '\n'
            << summary.best.accuracy << '\n';

  return EXIT_SUCCESS;
}
