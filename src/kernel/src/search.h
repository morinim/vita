/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_SEARCH_H)
#define      VITA_SRC_SEARCH_H

#include "kernel/adf.h"
#include "kernel/search.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/problem.h"
#include "kernel/team.h"
#include "kernel/vitafwd.h"

namespace vita
{
enum class evaluator_id
{
  count = 0, mae, rmae, mse, bin, dyn_slot, gaussian, undefined
};

enum class metric_flags : unsigned
{
  nothing = 0x0000,

  accuracy = 1 << 0,
  f1_score = 1 << 1,

  everything = 0xFFFF
};

///
/// \brief search for GP
///
/// \tparam T the type of individual used
/// \tparam ES the adopted evolution strategy
///
/// This class implements vita::search for GP symbolic regression
/// classification tasks.
///
template<class T = i_mep, template<class> class ES = std_es>
class src_search : public search<T, ES>
{
public:
  explicit src_search(src_problem &, metric_flags = metric_flags::nothing);

  template<class U> void arl(const U &);
  template<class U> void arl(const team<U> &);

  bool set_evaluator(evaluator_id, const std::string & = "");

private:  // Private support methods
  void calculate_metrics(const T &, model_measurements *) const;
  void dss(unsigned) const;
  template<class C> void log(const summary<T> &,
                             const distribution<fitness_t> &,
                             const C &, typename C::value_type, unsigned);
  void print_resume(bool, const model_measurements &) const;

  // NVI template methods
  virtual bool debug_nvi() const override;
  virtual summary<T> run_nvi(unsigned) override;
  virtual void tune_parameters_nvi() override;

private:  // Private data members
  // Preferred evaluator for symbolic regression.
  evaluator_id p_symre;

  // Preferred evaluator for classification.
  evaluator_id p_class;

  // Metrics we have to calculate during the search.
  metric_flags metrics;
};

#include "kernel/src/search.tcc"
}  // namespace vita

#endif  // Include guard
