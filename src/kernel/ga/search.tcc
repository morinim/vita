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

  tune_parameters_nvi();

  for (unsigned r(0); r < n; ++r)
  {
    evolution<T, ES> evo(this->env_, eval, nullptr, nullptr);
    summary<T> run_summary(evo.run(r));

    print_resume(run_summary.best.score.fitness);

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
    log(overall_summary, fd, good_runs, best_run, n);
  }

  return overall_summary;
}

///
/// \param[in] fit fitness reached in the current run.
///
template<class T, template<class> class ES, class F>
void ga_search<T, ES, F>::print_resume(const fitness_t &fit) const
{
  print.info("Fitness: ", fit, "\n\n");
}

///
/// \param[in] run_sum summary information regarding the search.
/// \param[in] fd statistics about population fitness.
/// \param[in] good_runs list of the best runs of the search.
/// \param[in] best_run best overall run.
/// \param[in] runs number of runs performed.
/// \return \c true if the write operation succeed.
///
/// Writes end-of-run logs (run summary, results for test...).
///
template<class T, template<class> class ES, class F>
template<class C>
void ga_search<T, ES, F>::log(const summary<T> &run_sum,
                              const distribution<fitness_t> &fd,
                              const C &good_runs,
                              typename C::value_type best_run, unsigned runs)
{
  if (!this->env_.stat.summary)
    return;

  // Summary logging.
  tinyxml2::XMLPrinter p;
  p.OpenElement("vita");

  p.OpenElement("summary");

  const auto solutions(static_cast<unsigned>(good_runs.size()));
  const auto success_rate(
    runs ? static_cast<double>(solutions) / static_cast<double>(runs)
         : 0);

  push_text(p, "success_rate", success_rate);

  push_text(p, "elapsed_time", run_sum.elapsed);
  push_text(p, "mean_fitness", fd.mean());
  push_text(p, "standard_deviation", fd.standard_deviation());

  p.OpenElement("best");

  push_text(p, "fitness", run_sum.best.score.fitness);
  push_text(p, "run", best_run);

  p.OpenElement("solution");

  std::ostringstream ss;
  run_sum.best.solution.list(ss);
  push_text(p, "list", ss.str());

  p.CloseElement();  // </solution>
  p.CloseElement();  // </best>

  p.OpenElement("solutions");
  p.OpenElement("runs");
  for (const auto &gr : good_runs)
    push_text(p, "run", gr);
  p.CloseElement();  // </runs>

  push_text(p, "found", solutions);

  const auto avg_depth(solutions ? run_sum.last_imp / solutions : 0);
  push_text(p, "avg_depth", avg_depth);

  p.CloseElement();  // </solution>

  p.OpenElement("other");
  push_text(p, "evaluator", this->active_eva_->info());
  p.CloseElement();  // </other>

  p.CloseElement();  // </summary>

  this->env_.xml(&p);

  p.CloseElement();  // </vita>

  const std::string f_sum(this->env_.stat.dir + "/" +
                          this->env_.stat.sum_name);
  std::ofstream of(f_sum);
  of << p.CStr();
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
