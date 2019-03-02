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

bool crossing(const maze &m, cell_coord pos)
{
  unsigned n(0);

  n += pos.first > 0                && m[pos.first - 1][pos.second] == Empty;
  n += pos.first + 1 < m.size()     && m[pos.first + 1][pos.second] == Empty;
  n += pos.second > 0               && m[pos.first][pos.second - 1] == Empty;
  n += pos.second + 1 < m[0].size() && m[pos.first][pos.second + 1] == Empty;

  return n > 2;
}

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

std::vector<cell_coord> extract_path(const vita::i_ga &dirs, const maze &m,
                                     cell_coord start, cell_coord goal)
{
  std::vector<cell_coord> ret;

  cell_coord now(start);

  for (unsigned i(0); i < dirs.size() && now != goal; ++i)
  {
    const auto dir(dirs[i]);
    cell_coord prev;
    do
    {
      prev = now;
      ret.push_back(now);
      now = update_coord(m, now, dir);
    } while (now != prev && now != goal && !crossing(m, now));

    if (now == goal)
      ret.push_back(goal);
  }

  return ret;
}

std::pair<cell_coord, unsigned> run(const vita::i_ga &dirs, const maze &m,
                                    cell_coord start, cell_coord goal)
{
  const auto path(extract_path(dirs, m, start, goal));

  return {path.back(), path.size()};
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

maze path_on_maze(const std::vector<cell_coord> &path, maze base,
                  cell_coord goal)
{
  for (const auto &c : path)
    base[c.first][c.second] = '.';

  base[path.front().first][path.front().second] = Start;

  if (path.back() == goal)
    base[goal.first][goal.second] = Goal;

  return base;
}

int main()
{
  using namespace vita;

  const cell_coord start{0, 0}, goal{16, 16};
  const maze m =
  {
    " *               ",
    " * *** * ********",
    "   *   *         ",
    " *** ********* * ",
    " *   *       * * ",
    " ***** ***** *** ",
    "   *       * *   ",
    "** * ***** * * * ",
    "   * *   * * * * ",
    "** * * * * * * * ",
    "   *   * * *   * ",
    " ******* ********",
    "       * *       ",
    "**** * * * ***** ",
    "   * * *   *   * ",
    " *** * ***** * * ",
    "     *       * * "
  };

  const auto length(m.size() * m[0].size() / 2);

  // A candidate solution is a sequence of `length` integers each representing
  // a cardinal direction.
  ga_problem prob(length, {0, 4});

  prob.env.individuals = 150;
  prob.env.generations =  20;

  auto f = [m, start, goal](const i_ga &x)
  {
    const auto final(run(x, m, start, goal));

    return -distance(final.first, goal) - final.second / 1000.0;
  };

  ga_search<decltype(f)> search(prob, f);

  const auto best_path(extract_path(search.run().best.solution, m, start,
                                    goal));

  print_maze(path_on_maze(best_path, m, goal));
}
