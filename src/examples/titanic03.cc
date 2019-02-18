/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/titanic_tutorial
 */

#include <cstdlib>

#include "kernel/vita.h"

int main()
{
  using namespace vita;

  src_problem titanic("titanic_train.csv", src_problem::default_symbols);

  if (!titanic)
    return EXIT_FAILURE;

  titanic.env.mep.code_length =  130;
  titanic.env.individuals     = 1000;
  titanic.env.generations     =  200;

  src_search<> s(titanic, metric_flags::accuracy);
  const auto summary(s.run(10));

  std::cout << summary.best.solution << '\n'
            << summary.best.score.accuracy << '\n';
}
