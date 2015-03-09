/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_SEARCH_H)
#define      VITA_GA_SEARCH_H

#include <list>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/version.hpp>

#include "kernel/ga/evaluator.h"
#include "kernel/search.h"

namespace vita
{
///
/// \brief search for GA
///
/// \tparam T the type of individual used
/// \tparam ES the adopted evolution strategy
///
/// This class implements vita::search for GA optimization tasks.
///
template<class T, template<class> class ES, class F>
class ga_search : public search<T, ES>
{
public:
  ga_search(problem &, F, penalty_func_t<T> = nullptr);

private:  // NVI template methods
  virtual bool debug_nvi(bool) const override;
  virtual T run_nvi(unsigned) override;
  virtual void tune_parameters_nvi() override;

private:  // Private support methods
  void log(const summary<T> &, const distribution<fitness_t> &,
           const std::list<unsigned> &, unsigned, unsigned);
  void print_resume(const fitness_t &) const;
};

#include "kernel/ga/search.tcc"
}  // namespace vita

#endif  // Include guard
