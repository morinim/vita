/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
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
  vita::src_problem problem("titanic_train.csv");  // reading training set

  if (!problem)
    return EXIT_FAILURE;

  vita::src_search<> s(&problem);
  const auto best(s.run());                        // starting search and
                                                   // getting best individual
  std::cout << best << std::endl;

  return EXIT_SUCCESS;
}
