/**
 *
 *  \file kernel/problem.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(PROBLEM_H)
#define      PROBLEM_H

#include <vector>

#include "kernel/data.h"
#include "kernel/environment.h"
#include "kernel/evaluator.h"
#include "kernel/symbol_set.h"

namespace vita
{
  class individual;
  class lambda_f;

  class problem
  {
  public:
    problem();

    evaluator *get_evaluator();
    void set_evaluator(std::unique_ptr<evaluator>);

    /// \return an access point for the dataset.
    virtual vita::data *data() { return nullptr; }

    virtual void clear();

    virtual std::unique_ptr<lambda_f> lambdify(const individual &) = 0;

    virtual bool debug(bool) const;

  public:  // Public data members.
    environment env;

    symbol_set sset;

  protected:  // Private data members.
    std::unique_ptr<evaluator> active_eva_;
  };
}  // namespace vita

#endif  // PROBLEM_H
