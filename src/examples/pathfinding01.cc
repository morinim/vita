/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/pathfinding_tutorial
 */

#include <iostream>
#include <vector>

#include "kernel/vita.h"

using maze = std::vector<std::string>;

enum cell {Start = 'S', Goal = 'G', Wall = '*', Empty = ' '};

using cell_coord = std::pair<unsigned, unsigned>;

// Taxicab distance.
double distance(cell_coord c1, cell_coord c2)
{
  return std::max(c1.first, c2.first) - std::min(c1.first, c2.first) +
         std::max(c1.second, c2.second) - std::min(c1.second, c2.second);
}

enum cardinal_dir {north, south, west, east};

cell_coord update_coord(const maze &m, cell_coord start, int d)
{
  Expects(d == north || d == south || d == west || d == east);
  auto to(start);

  switch(d)
  {
  case north:
    if (start.first > 0)
      --to.first;
    break;

  case south:
    if (start.first + 1 < m.size())
      ++to.first;
    break;

  case west:
    if (start.second > 0)
      --to.second;
    break;

  default:
    if (start.second + 1 < m[0].size())
      ++to.second;
  }

  return m[to.first][to.second] == Empty ? to : start;
}

std::pair<cell_coord, unsigned> run(const vita::i_ga &path, const maze &m,
                                    cell_coord start, cell_coord goal)
{
  cell_coord now(start);

  unsigned step(0);
  for (; step < path.parameters() && now != goal; ++step)
    now = update_coord(m, now, path[step]);

  return {now, step};
}

void print_maze(const maze &m)
{
  const std::string hr(m[0].size() + 2, '-');

  std::cout << hr << '\n';

  for (const auto &rows : m)
  {
    std::cout << '|';

    for (const auto &cell : rows)
      std::cout << cell;

    std::cout << "|\n";
  }

  std::cout << hr << '\n';
}

maze path_on_maze(const vita::i_ga &path, const maze &base,
                  cell_coord start, cell_coord goal)
{
  auto ret(base);
  auto now = start;

  for (const auto &dir : path)
  {
    auto &c = ret[now.first][now.second];

    if (now == start)
      c = Start;
    else if (now == goal)
    {
      c = Goal;
      break;
    }
    else
      c = '.';

    now = update_coord(base, now, dir);
  }

  return ret;
}

int main()
{
  using namespace vita;

  const cell_coord start{0, 0}, goal{16, 8};
  const maze m =
  {
    " *       ",
    " * *** * ",
    "   *   * ",
    " *** ****",
    " *   *   ",
    " ***** **",
    "   *     ",
    "** * ****",
    "   * *   ",
    "** * * * ",
    "   *   * ",
    " ******* ",
    "       * ",
    "**** * * ",
    "   * * * ",
    " *** * **",
    "     *   "
  };

  const auto length(m.size() * m[0].size());

  // A candidate solution is a sequence of `length` integers each representing
  // a cardinal direction.
  ga_problem prob(length, {0, 4});

  prob.env.individuals = 150;
  prob.env.generations =  20;

  // The fitness function.
  auto f = [m, start, goal](const i_ga &x)
  {
    const auto final(run(x, m, start, goal));

    return -distance(final.first, goal) - final.second / 1000.0;
  };

  ga_search<decltype(f)> search(prob, f);

  const auto best_path(search.run().best.solution);

  print_maze(path_on_maze(best_path, m, start, goal));
}
