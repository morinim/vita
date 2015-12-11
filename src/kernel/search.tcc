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

#if !defined(VITA_SEARCH_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_SEARCH_TCC)
#define      VITA_SEARCH_TCC

///
/// \param[in] p the problem we're working on. The lifetime of `p` must exceed
///              the lifetime of `this` class.
///
template<class T, template<class> class ES>
search<T, ES>::search(problem &p) : active_eva_(nullptr), env_(p.env),
                                    prob_(p), shake_(nullptr), stop_(nullptr)
{
  assert(debug());
}

///
/// \brief Tries to tune search parameters for the current problem
///
template<class T, template<class> class ES>
void search<T, ES>::tune_parameters()
{
  // The `shape` function modifies the default parameters with
  // strategy-specific values.
  const environment dflt(ES<T>::shape(environment(nullptr, true)));
  const environment &constrained(prob_.env);

  if (!constrained.code_length)
    env_.code_length = dflt.code_length;

  if (!constrained.patch_length)
    env_.patch_length = 1 + env_.code_length / 3;

  if (constrained.elitism == trilean::unknown)
    env_.elitism = dflt.elitism;

  if (constrained.p_mutation < 0.0)
    env_.p_mutation = dflt.p_mutation;

  if (constrained.p_cross < 0.0)
    env_.p_cross = dflt.p_cross;

  if (!constrained.brood_recombination)
    env_.brood_recombination = *dflt.brood_recombination;

  if (constrained.dss == trilean::unknown)
    env_.dss = trilean::no;

  if (!constrained.layers)
    env_.layers = dflt.layers;

  if (constrained.validation_percentage > 100)
    env_.validation_percentage = dflt.validation_percentage;

  if (!constrained.individuals)
    env_.individuals = dflt.individuals;

  if (!constrained.min_individuals)
    env_.min_individuals = dflt.min_individuals;

  if (!constrained.tournament_size)
    env_.tournament_size = dflt.tournament_size;

  if (!constrained.mate_zone)
    env_.mate_zone = *dflt.mate_zone;

  if (!constrained.generations)
    env_.generations = dflt.generations;

  if (!constrained.g_without_improvement)
    env_.g_without_improvement = dflt.g_without_improvement;

  if (constrained.arl == trilean::unknown)
    env_.arl = dflt.arl;

  assert(env_.debug(true));
}

///
/// \param[in] n number of runs.
/// \return a summary of the search.
///
template<class T, template<class> class ES>
summary<T> search<T, ES>::run(unsigned n)
{
  tune_parameters();

  preliminary_setup();

  summary<T> overall_summary;
  distribution<fitness_t> fd;

  unsigned best_run(0);
  std::vector<decltype(best_run)> good_runs;

  for (unsigned r(0); r < n; ++r)
  {
    auto run_summary(evolution<T, ES>(this->env_, *this->active_eva_,
                                      stop_,shake_).run(r));

    after_evolution(&run_summary);

    print_resume(run_summary.best.score);

    if (r == 0 ||
        run_summary.best.score.fitness > overall_summary.best.score.fitness)
    {
      overall_summary.best = run_summary.best;
      best_run = r;
    }

    // We use accuracy or fitness (or both) to identify successful runs.
    const bool solution_found(run_summary.best.score >=
                              this->env_.threshold);

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
/// \param[in] m metrics relative to the current run.
///
template<class T, template<class> class ES>
void search<T, ES>::print_resume(const model_measurements &m) const
{
  print.info("Fitness: ", m.fitness);
}

///
/// \param[in] ind individual to be transformed in a lambda function.
/// \return the lambda function associated with `ind` (`nullptr` in case of
///         errors).
///
/// The lambda function depends on the active evaluator.
///
template<class T, template<class> class ES>
std::unique_ptr<lambda_f<T>> search<T, ES>::lambdify(const T &ind) const
{
  return active_eva_->lambdify(ind);
}

///
/// \param[in] e the evaluator that should be set as active.
///
template<class T, template<class> class ES>
void search<T, ES>::set_evaluator(std::unique_ptr<evaluator<T>> e)
{
  if (env_.ttable_size)
    active_eva_ = vita::make_unique<evaluator_proxy<T>>(std::move(e),
                                                        env_.ttable_size);
  else
    active_eva_ = std::move(e);
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T, template<class> class ES>
bool search<T, ES>::debug() const
{
  if (!env_.debug(false))
    return false;

  return debug_nvi();
}

///
/// \param[in] run_sum summary information regarding the search.
/// \param[in] fd statistics about population fitness.
/// \param[in] good_runs list of the best runs of the search.
/// \param[in] best_run best overall run.
/// \param[in] runs number of runs performed.
/// \return `true` if the write operation succeed.
///
/// Writes end-of-run logs (run summary, results for test...).
///
template<class T, template<class> class ES>
void search<T, ES>::log(const summary<T> &run_sum,
                        const distribution<fitness_t> &fd,
                        const std::vector<unsigned> &good_runs,
                        unsigned best_run, unsigned runs) const
{
  // Summary logging.
  if (!env_.stat.summary)
    return;

  tinyxml2::XMLDocument d;

  auto *root(d.NewElement("vita"));
  d.InsertFirstChild(root);

  auto *e_summary(d.NewElement("summary"));
  root->InsertEndChild(e_summary);

  const auto solutions(static_cast<unsigned>(good_runs.size()));
  const auto success_rate(
    runs ? static_cast<double>(solutions) / static_cast<double>(runs)
         : 0);

  set_text(e_summary, "success_rate", success_rate);
  set_text(e_summary, "elapsed_time", run_sum.elapsed);
  set_text(e_summary, "mean_fitness", fd.mean());
  set_text(e_summary, "standard_deviation", fd.standard_deviation());

  auto *e_best(d.NewElement("best"));
  e_summary->InsertEndChild(e_best);
  set_text(e_best, "fitness", run_sum.best.score.fitness);
  set_text(e_best, "run", best_run);

  auto *e_solution(d.NewElement("solution"));
  e_best->InsertEndChild(e_solution);
  std::ostringstream ss;
  run_sum.best.solution.list(ss);
  set_text(e_solution, "list", ss.str());

  auto *e_solutions(d.NewElement("solutions"));
  e_summary->InsertEndChild(e_solutions);

  auto *e_runs(d.NewElement("runs"));
  e_solutions->InsertEndChild(e_runs);
  for (const auto &gr : good_runs)
    set_text(e_runs, "run", gr);
  set_text(e_solutions, "found", solutions);

  const auto avg_depth(solutions ? run_sum.last_imp / solutions : 0);
  set_text(e_solutions, "avg_depth", avg_depth);

  auto *e_other(d.NewElement("other"));
  e_summary->InsertEndChild(e_other);
  set_text(e_other,"evaluator", active_eva_->info());

  env_.xml(&d);

  log_nvi(&d, run_sum);

  const std::string f_sum(env_.stat.dir + "/" + env_.stat.sum_name);
  d.SaveFile(f_sum.c_str());
}
#endif  // Include guard
