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
 *  \see https://github.com/morinim/vita/wiki/string_guessing_tutorial
 */

#include <iostream>

#include "kernel/vita.h"

const int NQUEENS(8);

// Encodes the queen's placement (i.e. a gene in the chromosome).
struct row : vita::ga::integer
{
  row() : vita::ga::integer({0, NQUEENS}) {}  // open range [0; NQUEENS[
};

int main()
{
  vita::problem prob;
  prob.chromosome<row>(NQUEENS);

  // Fitness function.
  auto f = [](const vita::i_ga &x) -> vita::fitness_t
  {
    double attacks(0);

    for (int queen(0); queen < NQUEENS - 1; ++queen)
    {
      const int row(x[queen].as<int>());

      for (int i(queen + 1); i < NQUEENS; ++i)
      {
        const int other_row(x[i].as<int>());

        if (other_row == row                            // same row
            || std::abs(other_row - row) == i - queen)  // or diagonal
          ++attacks;
      }
    }

    return {-attacks};
  };

  // Let's go.
  vita::ga_search<decltype(f)> search(prob, f);
  auto result = search.run();

  // Prints result.
  std::cout << "\nBest result: [";
  for (auto gene : result.best.solution)
    std::cout << " " << gene.as<int>();

  std::cout << " ]   (fitness " << result.best.score.fitness << ")\n";
}
