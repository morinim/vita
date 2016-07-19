/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <chrono>
#include <vector>

#include "kernel/ga/evaluator.h"
#include "kernel/ga/i_de.h"
#include "kernel/ga/search.h"

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
  std::generate(job_duration.begin(), job_duration.end(),
                []
                {
                  return std::chrono::hours(vita::random::between(1, 4));
                });

  std::cout << "Total time required: "
            << std::accumulate(job_duration.begin(), job_duration.end(),
                               std::chrono::hours(0)).count()
            << '\n';

  vita::problem prob;
  prob.env.individuals =  250;
  prob.env.generations = 2000;

  // Problem's parameters.
  for (unsigned i(0); i < n_jobs; ++i)
    prob.env.sset->insert(vita::ga::parameter(i, -0.5, 23.5));

  vita::ga_search<vita::i_de, vita::de_es, decltype(&f)> search(prob, &f);

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

  return EXIT_SUCCESS;
}
