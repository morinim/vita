/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_SEARCH_H)
#define      VITA_SRC_SEARCH_H

#include "kernel/vitafwd.h"
#include "kernel/gp/src/dss.h"
#include "kernel/gp/src/evaluator.h"
#include "kernel/gp/src/holdout_validation.h"
#include "kernel/gp/src/problem.h"
#include "kernel/gp/team.h"
#include "kernel/search.h"

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
class src_search final : public search<T, ES>
{
public:
  using individual_type = T;

  explicit src_search(src_problem &, metric_flags = metric_flags::nothing);

  std::unique_ptr<basic_src_lambda_f> lambdify(const T &) const;

  src_search &evaluator(evaluator_id, const std::string & = "");
  src_search &validation_strategy(validator_id);

  bool is_valid() const override;

protected:
  // *** Template methods / customization points ***
  void after_evolution(const summary<T> &) override;

  void calculate_metrics(summary<T> *) const override;

  // Requires the availability of a validation function and of validation data.
  bool can_validate() const override;

  void log_stats(const search_stats<T> &,
                 tinyxml2::XMLDocument *) const override;

  void print_resume(const model_measurements &) const override;

  void tune_parameters() override;

private:
  dataframe &training_data() const;
  dataframe &test_data() const;
  dataframe &validation_data() const;
  src_problem &prob() const;
  template<class E, class... Args> void set_evaluator(Args && ...);
  bool stop_condition(const summary<T> &) const;

  // *** Private data members ***
  // Preferred evaluator for symbolic regression.
  evaluator_id p_symre;

  // Preferred evaluator for classification.
  evaluator_id p_class;

  // Metrics we have to calculate during the search.
  metric_flags metrics;
};

#include "kernel/gp/src/search.tcc"

}  // namespace vita

#endif  // include guard
