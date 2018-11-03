/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SEARCH_H)
#define      VITA_SEARCH_H

#include "kernel/evolution.h"
#include "kernel/problem.h"
#include "kernel/validation_strategy.h"

namespace vita
{
///
/// Search drives the evolution.
///
/// \tparam T  individual used
/// \tparam ES evolution strategy
///
/// The class offers a general / customizable search strategy.
///
/// \note
/// The "template template parameter" approach allows coordination between `T`
/// and `ES` to be handled by the search class, rather than in all the various
/// code that specializes search.
/// A clear description of this technique can be found in "C++ Common
/// Knowledge: Template Template Parameters" by Stephen Dewhurst
/// (<http://www.informit.com/articles/article.aspx?p=376878>).
///
template<class T, template<class> class ES>
class search
{
public:
  explicit search(problem &);

  summary<T> run(unsigned = 1);

  template<class E, class... Args> void set_training_evaluator(Args && ...);
  template<class E, class... Args> void set_validation_evaluator(Args && ...);

  template<class V, class... Args> void set_validator(Args && ...);

  virtual bool debug() const;

protected:
  // Support methods.
  void log_search(const summary<T> &, const distribution<fitness_t> &,
                  const std::vector<unsigned> &, unsigned, unsigned) const;
  bool load();
  bool save() const;
  virtual void tune_parameters();

  model_measurements calculate_metrics(evaluator<T> &,
                                       const summary<T> &) const;

  // Data members.
  std::unique_ptr<evaluator<T>> eva1_;  // fitness function for training
  std::unique_ptr<evaluator<T>> eva2_;  // fitness function for validation
  std::unique_ptr<validation_strategy> vs_;

  // Problem we're working on.
  problem &prob_;

private:
  /// Template method of the search::run() member function called exactly one
  /// time just before the first run.
  virtual void init() {}

  virtual void after_evolution(summary<T> *) {}
  virtual void print_resume(const model_measurements &) const;

  // Logs additional problem-specific data.
  virtual void log_search_spec(tinyxml2::XMLDocument *,
                               const summary<T> &) const
  {}

  virtual model_measurements calculate_metrics_spec(const summary<T> &) const;
};

#include "kernel/search.tcc"
}  // namespace vita

#endif  // include guard
