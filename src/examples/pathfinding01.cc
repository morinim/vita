/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://bitbucket.org/morinim/vita/wiki/path_finding_tutorial
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

// Encodes the i-th move of a path.
class move : public vita::ga::integer
{
public:
  enum cardinal_dir {north, south, west, east};

  explicit move(unsigned step) : vita::ga::integer(step, 0, 4)  {}

  std::string display(double v) const override
  {
    switch (static_cast<unsigned>(v))
    {
    case north: return "N";
    case south: return "S";
    case west:  return "W";
    default:    return "E";
    }
  }
};

cell_coord update_coord(const maze &m, cell_coord start, move::cardinal_dir d)
{
  auto to(start);

  switch(d)
  {
  case move::north:
    if (start.first > 0)
      --to.first;
    break;

  case move::south:
    if (start.first + 1 < m.size())
      ++to.first;
    break;

  case move::west:
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
    now = update_coord(m, now, path[step].as<move::cardinal_dir>());

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

  for (unsigned i(0); i < path.parameters(); ++i)
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

    now = update_coord(base, now, path[i].as<move::cardinal_dir>());
  }

  return ret;
}

int main()
{
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

  const std::size_t sup_length(m.size() * m[0].size());

  vita::problem prob;

  for (unsigned step(0); step < sup_length; ++step)
    prob.env.sset->insert(std::make_unique<move>(step));

  prob.env.individuals = 150;
  prob.env.generations =  20;

  // The fitness function.
  auto f = [m, start, goal](const vita::i_ga &x)
  {
    const auto final(run(x, m, start, goal));

    return -distance(final.first, goal) - final.second / 1000.0;
  };

  vita::ga_search<vita::i_ga, vita::std_es, decltype(f)> search(prob, f);

  const auto best_path(search.run().best.solution);

  print_maze(path_on_maze(best_path, m, start, goal));
}
