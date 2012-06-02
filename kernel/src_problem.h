/**
 *
 *  \file src_problem.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_PROBLEM_H)
#define      SRC_PROBLEM_H

#include <cmath>
#include <list>
#include <string>
#include <vector>

#include "kernel/problem.h"

namespace vita
{
  class src_problem : public problem
  {
  public:
    enum {k_abs_evaluator = 0, k_count_evaluator, k_dyn_slot_evaluator};

    explicit src_problem(fitness_t);

    unsigned load_data(const std::string &);
    unsigned load_symbols(const std::string &);
    void setup_default_symbols();

    vita::data *data() { return &dat_; }

    void clear();

    unsigned categories() const;
    unsigned classes() const;
    unsigned variables() const;

    bool check(bool) const;

  private:
    typedef std::vector<category_t> cvect;

    std::list<cvect> seq_with_rep(const cvect &, unsigned);
    bool compatible(const cvect &, const std::vector<std::string> &) const;

    std::vector<variable_ptr> vars_;
    vita::data                 dat_;
  };
}  // namespace vita

#endif  // SRC_PROBLEM_H
