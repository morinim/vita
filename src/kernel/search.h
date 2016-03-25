/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SEARCH_H)
#define      VITA_SEARCH_H

#include "kernel/evolution.h"
#include "kernel/lambda_f.h"
#include "kernel/problem.h"

namespace vita
{
///
/// \brief Search drives the evolution
///
/// \tparam T the type of individual used
/// \tparam ES the adopted evolution strategy
///
/// The class is an interface for specific search strategies. The design
/// adheres to the NVI pattern ("Virtuality" in C/C++ Users Journal September
/// 2001 - <http://www.gotw.ca/publications/mill18.htm>).
///
/// \note
/// The "template template parameter" approach allows coordination between `T`
/// and `ES` to be handled by the search class, rather than in all the various
/// code that specializes search.
/// A very interesting description of this technique can be found in
/// "C++ Common Knowledge: Template Template Parameters" by Stephen Dewhurst
/// (<http://www.informit.com/articles/article.aspx?p=376878>).
///
template<class T, template<class> class ES>
class search
{
public:
  explicit search(problem &);

  std::unique_ptr<lambda_f<T>> lambdify(const T &) const;

  summary<T> run(unsigned = 1);

  void set_evaluator(std::unique_ptr<evaluator<T>>);

  bool debug() const;

protected:
  // Protected support methods
  void log(const summary<T> &, const distribution<fitness_t> &,
           const std::vector<unsigned> &, unsigned, unsigned) const;
  virtual void tune_parameters();

  // Protected data members
  std::unique_ptr<evaluator<T>> active_eva_;

  // This is the environment actually used during the search (`prob_->env`
  // is used for compiling `env_` via the `tune_parameters` method).
  environment env_;

  // Problem we're working on.
  problem &prob_;

  std::function<void (unsigned)> shake_;
  std::function<bool (const summary<T> &)> stop_;

private:
  // Template methods for search::run() member function.
  virtual void preliminary_setup() {}
  virtual void after_evolution(summary<T> *) {}
  virtual void print_resume(const model_measurements &) const;

  // NVI template methods
  virtual bool debug_nvi() const { return true; }
  virtual void log_nvi(tinyxml2::XMLDocument *, const summary<T> &) const {}
};

#include "kernel/search.tcc"
}  // namespace vita

#endif  // Include guard
