/**
 *
 *  \file example9a.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>

#include "search.h"
#include "src_problem.h"

int main()
{
  vita::src_problem problem;

  if (problem.load("titanic_train.csv").first)  // reading data file
  {
    vita::search<> s(&problem);
    vita::individual best(s.run());             // starting search

    std::cout << best << std::endl;             // print result

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}
