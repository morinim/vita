/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
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

template<class T>
struct search_stats
{
  void update(const summary<T> &);

  summary<T> overall = {};
  distribution<fitness_t> fd = {};
  std::set<unsigned> good_runs = {};

  unsigned best_run = 0;  /// index of the run giving the best solution
  unsigned runs     = 0;  /// number of runs performed so far
};

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
template<class T, template<class> class ES = vita::std_es>
class search
{
public:
  explicit search(problem &);

  summary<T> run(unsigned = 1);

  template<class E, class... Args> search &training_evaluator(Args && ...);
  template<class E, class... Args> search &validation_evaluator(Args && ...);

  template<class V, class... Args> search &validation_strategy(Args && ...);

  search &after_generation(
    typename evolution<T, ES>::after_generation_callback_t);

  virtual bool is_valid() const;

protected:
  // Template method of the search::run() member function called at the end of
  // each run.
  virtual void after_evolution(const summary<T> &);

  virtual void calculate_metrics(summary<T> *) const;

  // Returns `true` when a validation criterion is available: i.e. it needs
  // that `eva2_` is set. Derived classes can add further requirements.
  virtual bool can_validate() const;

  // Template method of the search::run() member function called exactly one
  // time at the end of the last run.
  virtual void close();

  // Template method of the search::run() member function called exactly one
  // time just before the first run.
  virtual void init();

  // Template method of the search::run() member function called at the end of
  // each run. Logs search statistics.
  virtual void log_stats(const search_stats<T> &,
                         tinyxml2::XMLDocument *) const;

  // Template method of the search::after_evolution member function.
  virtual void print_resume(const model_measurements &) const;

  virtual void tune_parameters();

  // *** Data members ***
  std::unique_ptr<evaluator<T>> eva1_;  // fitness function for training
  std::unique_ptr<evaluator<T>> eva2_;  // fitness function for validation
  std::unique_ptr<vita::validation_strategy> vs_;

  // Problem we're working on.
  problem &prob_;

  // Callback functions.
  typename evolution<T, ES>::after_generation_callback_t
  after_generation_callback_;

private:
  void log_stats(const search_stats<T> &) const;
  bool load();
  bool save() const;
};

#include "kernel/search.tcc"
}  // namespace vita

#endif  // include guard
