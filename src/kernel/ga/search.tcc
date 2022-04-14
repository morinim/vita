/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_SEARCH_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_GA_SEARCH_TCC)
#define      VITA_GA_SEARCH_TCC

///
/// Search class specialization for GAs / DE.
///
template<class T, template<class> class ES, class F>
basic_ga_search<T, ES, F>::basic_ga_search(problem &pr, F f,
                                           penalty_func_t<T> pf)
  : search<T, ES>(pr)
{
  if (pf)
    search<T, ES>::template training_evaluator<
      constrained_evaluator<T, ga_evaluator<T, F>, penalty_func_t<T>>>(
        ga_evaluator<T, F>(f), pf);
  else
    search<T, ES>::template training_evaluator<ga_evaluator<T, F>>(f);
}

///
/// Tries to tune search parameters for the current function.
///
/// \see src_search::tune_parameters_nvi comments for further details.
///
template<class T, template<class> class ES, class F>
void basic_ga_search<T, ES, F>::tune_parameters()
{
  search<T, ES>::tune_parameters();

  if (this->prob_.env.min_individuals < 10)
    this->prob_.env.min_individuals = 10;

  Ensures(this->prob_.env.is_valid(true));
}

#endif  // include guard
