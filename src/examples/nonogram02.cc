/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2018-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/nonogram_tutorial
 */

#include <iostream>
#include <string>

#include "kernel/vita.h"

struct nonogram_problem
{
  nonogram_problem(const std::vector<std::vector<unsigned>> &r_clues,
                   const std::vector<std::vector<unsigned>> &c_clues)
    : row_clues(r_clues), col_clues(c_clues)
  {
  }

  std::size_t rows() const { return row_clues.size(); }
  std::size_t cols() const { return col_clues.size(); }

  unsigned blocks() const
  {
    return std::accumulate(col_clues.begin(), col_clues.end(), 0u,
                           [](auto sum, const auto &v)
                           {
                             return sum + v.size();
                           });

  }

  vita::matrix<bool> board(const vita::i_ga &x) const
  {
    const auto col_size(rows());

    unsigned index(0);  // index into `x`'s genome

    vita::matrix<bool> ret(rows(), cols());
    for (unsigned col(0); col < cols(); ++col)
      for (std::size_t block(0), start(0);
           block < col_clues[col].size();
           ++block)
      {
        // Size of the current block.
        auto block_size(col_clues[col][block]);
        // Iterator referring to the first of the remaining blocks.
        auto next(std::next(col_clues[col].begin(), block + 1));
        // Stop iterator.
        auto end(col_clues[col].end());
        // Every remaining block requires one space.
        auto spaces(std::distance(next, end));
        // Minimum number of cells reserved for the remaing blocks.
        auto reserved(std::accumulate(next, end, 0u) + spaces);
        // Number of allowed positions for the current block.
        auto allowed(col_size - reserved - start - block_size + 1);
        // Starting position of the current block.
        auto placed(start + x[index] % allowed);

        for (unsigned i(0); i < block_size; ++i)
          ret(placed + i, col) = true;

        ++index;
        start = placed + block_size + 1;
      }

    return ret;
  }

  const std::vector<std::vector<unsigned>> row_clues;
  const std::vector<std::vector<unsigned>> col_clues;
};

const nonogram_problem np(
  {
    {2,9},{2,10},{2,11},{2,12},{3,13},{3,13},{8,14},{8,14},{9,14},{2,11,14},{29},{29},{29},{29},{29},{29},{16,13},{16,13},{16,12},{14,12},{14,11},{14,10},{15,9},{15,7},{15,4},{16,3,2},{16,5},{18},{5,5},{3}
  },
  {
    {5},{1,14},{2,15},{2,14},{3,17},{4,19},{23},{22},{21},{22},{22},{22},{22},{21},{10,7},{9,7},{10,6},{14,5},{17,5},{19,4},{21,2},{23,1},{23,3},{24,5},{24,5},{24,5},{25,3},{25},{26},{26}
  }
);


void print_solution(const vita::i_ga &x)
{
  const auto board(np.board(x));

  for (unsigned row(0); row < np.rows(); ++row)
  {
    for (unsigned col(0); col < np.cols(); ++col)
    {
      std::cout << ' ';

      if (board(row, col))
        std::cout << '#';
      else
        std::cout << ' ';
    }

    std::cout << '\n';
  }
}

int main()
{
  using namespace vita;

  // A candidate solution is a sequence of `np.blocks()` integers in the
  // `[0, np.rows()[` interval.
  ga_problem prob(np.blocks(), {0u, np.rows()});

  prob.env.individuals = 30000;
  prob.env.generations =   500;

  // The fitness function.
  auto f = [](const i_ga &x) -> fitness_t
  {
    const auto board(np.board(x));
    double delta(0.0);

    for (unsigned row(0); row < np.rows(); ++row)
    {
      std::vector<unsigned> blocks;

      for (unsigned start(0); start < np.cols(); )
      {
        for (; start < np.cols(); ++start)
          if (board(row, start))
            break;

        if (start < np.cols())
        {
          unsigned end(start + 1);
          for (; end < np.cols(); ++end)
            if (!board(row, end))
              break;

          blocks.push_back(end - start);

          start = end;
        }
      }

      const auto clues(np.row_clues[row].size());
      for (unsigned i(0); i < std::max(blocks.size(), clues); ++i)
      {
        int v1(0), v2(0);

        if (i < clues)
          v1 = np.row_clues[row][i];
        if (i < blocks.size())
          v2 = blocks[i];

        delta += std::abs(v1 - v2);
      }
    }

    return {-delta};
  };

  ga_search<decltype(f)> search(prob, f);
  auto result = search.run(10);

  std::cout << "\nBest result\n";
  print_solution(result.best.solution);
  std::cout << result.best.score.fitness << '\n';
}
