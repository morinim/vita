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

#include <list>

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

///
/// \brief search for GP
///
/// \tparam T the type of individual used
/// \tparam ES the adopted evolution strategy
///
/// This class implements vita::search for GP symbolic regression /
/// classification tasks.
///
template<class T = i_mep, template<class> class ES = std_es>
class src_search : public search<T, ES>
{
public:
  explicit src_search(src_problem &);

  template<class U> void arl(const U &);
  template<class U> void arl(const team<U> &);

  bool set_evaluator(evaluator_id, const std::string & = "");

private:  // NVI template methods
  virtual bool debug_nvi(bool) const override;
  virtual T run_nvi(unsigned) override;
  virtual void tune_parameters_nvi() override;

private:  // Private support methods
  double accuracy(const T &) const;
  void dss(unsigned) const;
  void log(const summary<T> &, const distribution<fitness_t> &,
           const std::list<unsigned> &, unsigned, double, unsigned);
  void print_resume(bool, const fitness_t &, double) const;

private:  // Private data members
  // Preferred evaluator for symbolic regression.
  evaluator_id p_symre;

  // Preferred evaluator for classification.
  evaluator_id p_class;
};

#include "kernel/src/search.tcc"
}  // namespace vita

#endif  // Include guard
