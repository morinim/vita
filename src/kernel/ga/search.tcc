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
void ga_search<T, ES, F>::tune_parameters_nvi()
{
  const environment dflt(nullptr, true);
  const environment &constrained(this->prob_.env);

  if (constrained.p_mutation < 0.0)
    this->env_.p_mutation = dflt.p_mutation;

  if (constrained.p_cross < 0.0)
    this->env_.p_cross = dflt.p_cross;

  if (!constrained.tournament_size)
    this->env_.tournament_size = dflt.tournament_size;

  if (!constrained.mate_zone)
    this->env_.mate_zone = *dflt.mate_zone;

  if (!constrained.generations)
    this->env_.generations = dflt.generations;

  if (!constrained.g_without_improvement)
    this->env_.g_without_improvement = dflt.g_without_improvement;

  assert(this->env_.debug(true));
}

///
/// \param[in] n number of runs.
/// \return a summary of the search.
///
template<class T, template<class> class ES, class F>
summary<T> ga_search<T, ES, F>::run_nvi(unsigned n)
{
  auto &eval(*this->active_eva_);  // just a shorthand

  summary<T> overall_summary;
  distribution<fitness_t> fd;

  unsigned best_run(0);
  std::vector<unsigned> good_runs;

  for (unsigned r(0); r < n; ++r)
  {
    auto run_summary(evolution<T, ES>(this->env_, eval).run(r));

    this->print_resume("", run_summary.best.score);

    if (r == 0 ||
        run_summary.best.score.fitness > overall_summary.best.score.fitness)
    {
      overall_summary.best = run_summary.best;
      best_run = r;
    }

    // We use fitness to identify successful runs.
    const bool solution_found(dominating(run_summary.best.score.fitness,
                                         this->env_.threshold.fitness));

    if (solution_found)
    {
      overall_summary.last_imp += run_summary.last_imp;

      good_runs.push_back(r);
    }

    if (isfinite(run_summary.best.score.fitness))
      fd.add(run_summary.best.score.fitness);

    overall_summary.elapsed += run_summary.elapsed;

    assert(good_runs.empty() ||
           std::find(std::begin(good_runs), std::end(good_runs), best_run) !=
           std::end(good_runs));

    this->log(overall_summary, fd, good_runs, best_run, n);
  }

  return overall_summary;
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T, template<class> class ES, class F>
bool ga_search<T, ES, F>::debug_nvi() const
{
  return true;
}
#endif  // Include guard
