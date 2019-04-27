/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2019 EOS di Manlio Morini.
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
#include "kernel/src/dss.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/holdout_validation.h"
#include "kernel/src/problem.h"
#include "kernel/team.h"
#include "kernel/vitafwd.h"

namespace vita
{
enum class evaluator_id
{
  count = 0, mae, rmae, mse, bin, dyn_slot, gaussian, undefined
};
enum class validator_id
{
  as_is, dss, holdout, undefined
};

enum class metric_flags : unsigned
{
  nothing = 0x0000,

  accuracy = 1 << 0,
  f1_score = 1 << 1,

  everything = 0xFFFF
};

///
/// Drives the search for solutions of symbolic regression / classification
/// tasks.
///
/// \tparam T  the type of individual used
/// \tparam ES the adopted evolution strategy
///
template<class T = i_mep, template<class> class ES = std_es>
class src_search : public search<T, ES>
{
public:
  using individual_type = T;

  explicit src_search(src_problem &, metric_flags = metric_flags::nothing);

  std::unique_ptr<basic_src_lambda_f> lambdify(const T &) const;

  template<class U> void arl(const U &);
  template<class U> void arl(const team<U> &);

  src_search &evaluator(evaluator_id, const std::string & = "");
  src_search &validation_strategy(validator_id);

  bool debug() const override;

protected:
  void tune_parameters() override;

private:
  dataframe &training_data() const;
  dataframe &test_data() const;
  dataframe &validation_data() const;
  src_problem &prob() const;
  template<class E, class... Args> void set_evaluator(Args && ...);
  bool stop_condition(const summary<T> &) const;

  // *** Template methods / customization points for `search::run()` ***
  void after_evolution(summary<T> *) override;
  model_measurements calculate_metrics_custom(
    const summary<T> &) const override;

  // Requires the availability of a validation function and of validation data.
  bool can_validate() const override;

  void log_search_custom(tinyxml2::XMLDocument *,
                         const summary<T> &) const override;
  void print_resume(const model_measurements &) const override;

  // *** Private data members ***
  // Preferred evaluator for symbolic regression.
  evaluator_id p_symre;

  // Preferred evaluator for classification.
  evaluator_id p_class;

  // Metrics we have to calculate during the search.
  metric_flags metrics;
};

#include "kernel/src/search.tcc"

}  // namespace vita

#endif  // include guard
