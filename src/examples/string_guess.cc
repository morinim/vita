/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/string_guessing_tutorial
 */

#include <iostream>
#include <string>

#include "kernel/vita.h"

const std::string target = "Hello World";
const std::string CHARSET =
  " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!";

// The fitness function.
vita::fitness_t f(const vita::i_ga &x)
{
  double found(0);

  for (std::size_t i = 0; i < target.length(); ++i)
    if (target[i] == CHARSET[x[i]])
      ++found;

  return {found};
}

int main()
{
  // A solution of this problem is a fixed length (`target.length()`) string of
  // characters in a given charset (`CHARSET`).
  vita::ga_problem prob(target.length(), {0, CHARSET.size()});

  prob.env.individuals = 300;

  vita::ga_search<decltype(f)> search(prob, f);
  auto result = search.run();

  std::cout << "\nBest result: ";
  for (auto gene : result.best.solution)
    std::cout << CHARSET[gene];

  std::cout << " (fitness " << result.best.score.fitness << ")\n";
}
