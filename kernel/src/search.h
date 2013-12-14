/**
 *
 *  \file src_search.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_SEARCH_H)
#define      SRC_SEARCH_H

#include "kernel/search.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/problem.h"

namespace vita
{
  enum evaluator_id {k_count_evaluator = 0, k_mae_evaluator,
                     k_rmae_evaluator, k_mse_evaluator, k_bin_evaluator,
                     k_dyn_slot_evaluator, k_gaussian_evaluator,
                     k_sup_evaluator};

  ///
  /// \tparam T the type of individual used.
  /// \tparam ES the adopted evolution strategy.
  ///
  /// This class extends vita::search to simply manage evaluators for
  /// symbolic regression and classification.
  ///
  template<class T, template<class> class ES>
  class src_search : public search<T, ES>
  {
  public:
    explicit src_search(src_problem *const);

    void set_evaluator(evaluator_id, const std::string & = "");

    virtual bool debug(bool) const override;

  private:  // Private data members.
    // Preferred evaluator for symbolic regression.
    evaluator_id p_symre;

    // Preferred evaluator for classification.
    evaluator_id p_class;
  };

#include "kernel/src/search_inl.h"
}  // namespace vita

#endif  // SRC_SEARCH_H
