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
#include "kernel/ga/i_ga.h"
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
    if (start[i] + job_duration[i].count() >= 24.0)
      ret -= start[i] + job_duration[i].count() - 24.0;

    int occupied(0);
    for (unsigned j(0); j < start.size(); ++j)
      if (j != i &&
          start[j] < start[i] &&
          start[j] + job_duration[j].count() > start[i])
        occupied += 1;

    if (n_machines < occupied + 1)
      ret -= occupied + 1 - n_machines;
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

  vita::symbol_set ss;
  vita::environment env(&ss);

  env.individuals =  250;
  env.generations = 200;
  env.threshold.fitness = {0};

  vita::problem prob;
  prob.env = env;

  // Problem's parameters.
  for (unsigned i(0); i < n_jobs; ++i)
    prob.env.sset->insert(vita::ga::parameter(i, 0.0, 24.0));

  vita::ga_search<vita::i_ga, vita::de_es, decltype(&f)> search(prob, &f);

  const auto res(search.run().best.solution);

  for (unsigned i(0); i < n_jobs; ++i)
    std::cout << i << ' ' << std::round(res[i]) << ' '
              << job_duration[i].count() << '\n';

  return EXIT_SUCCESS;
}
