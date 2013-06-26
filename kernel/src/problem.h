/**
 *
 *  \file src/problem.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_PROBLEM_H)
#define      SRC_PROBLEM_H

#include <list>
#include <string>
#include <vector>

#include "kernel/problem.h"

namespace vita
{
  class src_problem : public problem
  {
  public:
    enum evaluator_id {k_count_evaluator = 0, k_sae_evaluator, k_sse_evaluator,
                       k_dyn_slot_evaluator, k_gaussian_evaluator,
                       k_max_evaluator = k_gaussian_evaluator};

    src_problem();

    std::pair<size_t, size_t> load(const std::string &,
                                   const std::string & = "",
                                   const std::string & = "");
    size_t load_symbols(const std::string &);
    size_t load_test_set(const std::string &);
    void setup_default_symbols();

    void set_evaluator(evaluator_id, const std::string & = "");

    virtual vita::data *data() { return &dat_; }
    virtual std::unique_ptr<lambda_f> lambdify(const individual &);

    virtual void clear();

    /// Just a shortcut for checking number of classes.
    bool classification() { return classes() > 1; }

    size_t categories() const;
    size_t classes() const;
    size_t variables() const;

    virtual bool debug(bool) const;

  public:  // Public data members.
    // Preferred evaluator for symbolic regression.
    evaluator_id p_symre;

    // Preferred evaluator for classification.
    evaluator_id p_class;

  private:
    typedef std::vector<category_t> cvect;

    std::list<cvect> seq_with_rep(const cvect &, size_t);
    bool compatible(const cvect &, const std::vector<std::string> &) const;
    void setup_terminals_from_data();

  private:  // Private data members.
    vita::data dat_;
  };
}  // namespace vita

#endif  // SRC_PROBLEM_H
