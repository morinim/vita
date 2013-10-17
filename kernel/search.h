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
#include "kernel/team.h"

namespace vita
{
  template<class T> class distribution;

  ///
  /// \tparam T the type of individuals used.
  /// \tparam ES the adopted evolution strategy.
  ///
  /// This \c class drives the evolution.
  ///
  /// \note
  /// The class uses a template template parameter.
  /// This approach allows coordination between T and ES to be handled by the
  /// basic_search class, rather than in all the various code that specializes
  /// basic_search.
  /// A very interesting description of this technique can be found in
  /// "C++ Common Knowledge: Template Template Parameters" by Stephen Dewhurst
  /// (<http://www.informit.com/articles/article.aspx?p=376878>).
  ///
  template<class T, template<class> class ES>
  class basic_search
  {
  public:
    explicit basic_search(problem *const);

    void arl(const T &);
    void arl(const basic_team<T> &);

    void tune_parameters();

    T run(unsigned = 1);

    bool debug(bool) const;

  private:  // Private support methods.
    double accuracy(const T &) const;
    void dss(unsigned) const;
    fitness_t fitness(const T &);
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

  using search = basic_search<individual, basic_alps_es>;

#include "kernel/search_inl.h"
}  // namespace vita

#endif  // SEARCH_H
