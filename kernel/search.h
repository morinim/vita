/**
 *
 *  \file search.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SEARCH_H)
#define      SEARCH_H

#include "kernel/vita.h"
#include "kernel/environment.h"
#include "kernel/fitness.h"

namespace vita
{
  template <class T> class distribution;
  class evolution;
  class individual;
  class problem;
  class summary;

  ///
  /// This \c class drives the evolution.
  ///
  class search
  {
  public:
    explicit search(problem *const);

    void arl(const individual &, evolution &);
    void tune_parameters();

    const individual &run(bool = true, unsigned = 1);

    bool check(bool) const;

  private:
    void dss(unsigned) const;
    void log(const summary &, const distribution<fitness_t> &,
             const std::list<unsigned> &, unsigned, unsigned) const;
    bool stop_condition(const summary &) const;

    environment env_;
    problem   *prob_;
  };
}  // namespace vita

#endif  // SEARCH_H
