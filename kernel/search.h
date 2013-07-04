/**
 *
 *  \file search.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SEARCH_H)
#define      SEARCH_H

#include <list>

#include <boost/property_tree/xml_parser.hpp>

#include "kernel/adf.h"
#include "kernel/evolution.h"
#include "kernel/lambda_f.h"
#include "kernel/problem.h"

namespace vita
{
  template<class T> class distribution;

  ///
  /// This \c class drives the evolution.
  ///
  template<class ES = basic_alps_es<individual>>
  class basic_search
  {
  public:
    typedef typename ES::individual_t individual_t;

    explicit basic_search(problem *const);

    void arl(const individual_t &);
    void tune_parameters();

    individual_t run(unsigned = 1);

    bool debug(bool) const;

  private:  // Private support methods.
    double accuracy(const individual_t &) const;
    void dss(unsigned) const;
    fitness_t fitness(const individual_t &);
    void log(const summary<individual_t> &, const distribution<fitness_t> &,
             const std::list<unsigned> &, unsigned, double, unsigned);
    void print_resume(bool, const fitness_t &, double) const;
    bool stop_condition(const summary<individual_t> &) const;

  private:  // Private data members.
    /// This is the environment actually used during the search (\a prob_->env
    /// is used for compiling \a env_ via the tune_parameters method).
    environment env_;
    problem   *prob_;
  };

  typedef basic_search<> search;

#include "kernel/search_inl.h"
}  // namespace vita

#endif  // SEARCH_H
