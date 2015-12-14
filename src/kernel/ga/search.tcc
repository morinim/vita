/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014, 2015 EOS di Manlio Morini.
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

template<class T, template<class> class ES, class F>
ga_search<T, ES, F>::ga_search(problem &pr, F f, penalty_func_t<T> pf)
  : search<T, ES>(pr)
{
  auto base_eva(vita::make_unique<ga_evaluator<T, F>>(f));

  if (pf)
  {
    auto eva(vita::make_unique<
	  constrained_evaluator<T, ga_evaluator<T, F>,
                            penalty_func_t<T>>>(*base_eva, pf));
    search<T, ES>::set_evaluator(std::move(eva));
  }
  else
    search<T, ES>::set_evaluator(std::move(base_eva));
}

///
/// \brief Tries to tune search parameters for the current function
///
/// \see src_search::tune_parameters_nvi comments for further details
///
template<class T, template<class> class ES, class F>
void ga_search<T, ES, F>::tune_parameters()
{
  search<T, ES>::tune_parameters();

  const environment dflt(nullptr, true);

  if (this->env_.min_individuals < 10)
    this->env_.min_individuals = 10;

  if (this->env_.arl != trilean::no)
  {
    this->env_.arl = trilean::no;
    print.info("ARL set to false");
  }

  assert(this->env_.debug(true));
}
#endif  // Include guard
