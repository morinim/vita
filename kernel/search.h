/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SEARCH_H)
#define      VITA_SEARCH_H

#include <list>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/version.hpp>

#include "kernel/adf.h"
#include "kernel/evolution.h"
#include "kernel/lambda_f.h"
#include "kernel/problem.h"
#include "kernel/team.h"

namespace vita
{
  template<class> class distribution;

  ///
  /// \brief This \c class drives the evolution
  ///
  /// \tparam T the type of individual used
  /// \tparam ES the adopted evolution strategy
  ///
  /// \note
  /// The class uses a template template parameter.
  /// This approach allows coordination between T and ES to be handled by the
  /// search class, rather than in all the various code that specializes
  /// search.
  /// A very interesting description of this technique can be found in
  /// "C++ Common Knowledge: Template Template Parameters" by Stephen Dewhurst
  /// (<http://www.informit.com/articles/article.aspx?p=376878>).
  ///
  template<class T, template<class> class ES>
  class search
  {
  public:
    explicit search(problem *const);

    template<class U> void arl(const U &);
    template<class U> void arl(const team<U> &);

    void tune_parameters();

    void set_evaluator(std::unique_ptr<evaluator<T>>);
    virtual std::unique_ptr<lambda_f<T>> lambdify(const T &);

    T run(unsigned = 1);

    virtual bool debug(bool) const;

  private:  // Private support methods
    double accuracy(const T &) const;
    void dss(unsigned) const;
    fitness_t fitness(const T &);
    void log(const summary<T> &, const distribution<fitness_t> &,
             const std::list<unsigned> &, unsigned, double, unsigned);
    void print_resume(bool, const fitness_t &, double) const;
    bool stop_condition(const summary<T> &) const;

  protected:  // Protected data members
    std::unique_ptr<evaluator<T>> active_eva_;

    /// This is the environment actually used during the search (\a prob_->env
    /// is used for compiling \a env_ via the tune_parameters method).
    environment env_;

    problem *prob_;
  };

#include "kernel/search_inl.h"
}  // namespace vita

#endif  // Include guard
