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

#include "adf.h"
#include "evolution.h"
#include "lambda_f.h"
#include "problem.h"

namespace vita
{
  template<class T> class distribution;

  ///
  /// This \c class drives the evolution.
  ///
  template<class T = individual, class ES = alps_es<T>>
  class basic_search
  {
  public:
    explicit basic_search(problem *const);

    void arl(const T &, evolution<T> &);
    void tune_parameters();

    T run(unsigned = 1);

    bool debug(bool) const;

  private:  // Private support methods.
    double accuracy(const T &) const;
    void dss(unsigned) const;
    void log(const summary<T> &, const distribution<fitness_t> &,
             const std::list<unsigned> &, unsigned, double, unsigned);
    void print_resume(bool, const fitness_t &, double) const;
    bool stop_condition(const summary<T> &) const;

  private:  // Private data members.
    /// This is the environment actually used during the search (\a prob_->env
    /// is used for compiling \a env_ via the tune_parameters method).
    environment env_;
    problem   *prob_;
  };

  typedef basic_search<> search;

#include "search_inl.h"
}  // namespace vita

#endif  // SEARCH_H
