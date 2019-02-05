/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_SEARCH_H)
#define      VITA_GA_SEARCH_H

#include "kernel/ga/evaluator.h"
#include "kernel/ga/problem.h"
#include "kernel/search.h"

namespace vita
{
///
/// Search driver for GAs.
///
/// \tparam T  the type of individual used
/// \tparam ES the adopted evolution strategy
///
/// This class implements vita::search for GA optimization tasks.
///
template<class T, template<class> class ES, class F>
class basic_ga_search : public search<T, ES>
{
public:
  basic_ga_search(problem &, F, penalty_func_t<T> = nullptr);

protected:
  void tune_parameters() override;
};

template<class F> using ga_search = basic_ga_search<i_ga, std_es, F>;
template<class F> using de_search = basic_ga_search<i_de, de_es, F>;

#include "kernel/ga/search.tcc"
}  // namespace vita

#endif  // include guard
