/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_SEARCH_H)
#define      VITA_SRC_SEARCH_H

#include "kernel/search.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/problem.h"
#include "kernel/vitafwd.h"

namespace vita
{
  enum class evaluator_id
  {
    count = 0, mae, rmae, mse, bin, dyn_slot, gaussian, undefined
  };

  ///
  /// \tparam T the type of individual used.
  /// \tparam ES the adopted evolution strategy.
  ///
  /// This class extends vita::search to simply manage evaluators for
  /// symbolic regression and classification.
  ///
  template<class T = i_mep, template<class> class ES = std_es>
  class src_search : public search<T, ES>
  {
  public:
    explicit src_search(src_problem *const);

    bool set_evaluator(evaluator_id, const std::string & = "");

    virtual bool debug(bool) const override;

  private:  // Private data members.
    // Preferred evaluator for symbolic regression.
    evaluator_id p_symre;

    // Preferred evaluator for classification.
    evaluator_id p_class;
  };

#include "kernel/src/search_inl.h"
}  // namespace vita

#endif  // Include guard
