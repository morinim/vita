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
 *  \see https://github.com/morinim/vita/wiki/scheduling_tutorial
 */

#include <chrono>
#include <vector>

#include "kernel/vita.h"

// Examples taken from "Differential Evolution in Discrete Optimization" by
// Daniel Lichtblau.

//`n_machines` homogeneous machines (i.e. each job time is independent of the
// machine used.
const int n_machines = 5;

// `n_jobs` with random durations.
const int n_jobs = 50;
std::vector<std::chrono::hours> job_duration(n_jobs);

// Assuming a total time period of one day.
double f(std::vector<double> start)
{
  double ret(0.0);

  for (auto &s : start)
    s = std::round(s);

  for (unsigned i(0); i < start.size(); ++i)
  {
    // A job starts at a nonnegative time.
    if (start[i] < 0.0)
      ret += start[i];

    // A job starts more than its length prior to the 24 hour limit.
    const auto end(start[i] + job_duration[i].count());
    if (end >= 24.0)
      ret -= end - 24.0;

    int occupied(1);
    for (unsigned j(0); j < start.size(); ++j)
      if (j != i &&
          start[j] <= start[i] &&
          start[j] + job_duration[j].count() > start[i])
        ++occupied;

    if (occupied > n_machines)
      ret -= occupied - n_machines;
  }

  return ret;
}

int main()
{
  using namespace vita;

  std::generate(job_duration.begin(), job_duration.end(),
                []
                {
                  return std::chrono::hours(random::between(1, 4));
                });

  std::cout << "Total time required: "
            << std::accumulate(job_duration.begin(), job_duration.end(),
                               std::chrono::hours(0)).count()
            << '\n';

  // A candidate solution is a sequence of `n_jobs` double in the
  // `[-0.5, 23.5[` interval.
  de_problem prob(n_jobs, {-0.5, 23.5});

  prob.env.individuals =  250;
  prob.env.generations = 2000;

  de_search<decltype(f)> search(prob, f);

  const auto res(search.run().best.solution);

  for (unsigned i(0); i < n_jobs; ++i)
    std::cout << i << ' ' << std::round(res[i]) << ' '
              << job_duration[i].count() << '\n';

  // A simple script for GnuPlot:
  // set xtics 1
  // set ytics 2
  // grid xtics ytics
  // plot [x=0:24][y=-0.5:50.5] "test.dat" using 2:1:3:(0)
  //      w vectors head filled lw 2 notitle
}
