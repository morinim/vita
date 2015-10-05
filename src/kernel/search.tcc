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
search<T, ES>::search(problem &p) : active_eva_(nullptr), env_(p.env), prob_(p)
{
  assert(debug());
}

template<class T, template<class> class ES>
fitness_t search<T, ES>::fitness(const T &ind)
{
  return (*active_eva_)(ind);
}

///
/// \param[in] n number of runs.
/// \return a summary of the search.
///
template<class T, template<class> class ES>
summary<T> search<T, ES>::run(unsigned n)
{
  tune_parameters_nvi();

  return run_nvi(n);
}

///
/// \param[in] s an up to date run summary.
/// \return `true` when a run should be interrupted.
///
template<class T, template<class> class ES>
bool search<T, ES>::stop_condition(const summary<T> &s) const
{
  // We use an accelerated stop condition when
  // * all the individuals have the same fitness
  // * after env_.g_without_improvement generations the situation doesn't
  //   change.
  assert(env_.g_without_improvement);
  if (s.gen - s.last_imp > env_.g_without_improvement &&
      issmall(s.az.fit_dist().variance()))
    return true;

  return false;
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
/// \param[out] d output xml document.
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
template<class C>
void search<T, ES>::log(tinyxml2::XMLDocument *d,
                        const summary<T> &run_sum,
                        const distribution<fitness_t> &fd,
                        const C &good_runs,
                        typename C::value_type best_run, unsigned runs)
{
  // Summary logging.
  if (!this->env_.stat.summary)
    return;

  auto *root(d->NewElement("vita"));
  d->InsertFirstChild(root);

  auto *e_summary(d->NewElement("summary"));
  root->InsertEndChild(e_summary);

  const auto solutions(static_cast<unsigned>(good_runs.size()));
  const auto success_rate(
    runs ? static_cast<double>(solutions) / static_cast<double>(runs)
         : 0);

  set_text(e_summary, "success_rate", success_rate);
  set_text(e_summary, "elapsed_time", run_sum.elapsed);
  set_text(e_summary, "mean_fitness", fd.mean());
  set_text(e_summary, "standard_deviation", fd.standard_deviation());

  auto *e_best(d->NewElement("best"));
  e_summary->InsertEndChild(e_best);
  set_text(e_best, "fitness", run_sum.best.score.fitness);
  set_text(e_best, "run", best_run);

  auto *e_solution(d->NewElement("solution"));
  e_best->InsertEndChild(e_solution);
  std::ostringstream ss;
  run_sum.best.solution.list(ss);
  set_text(e_solution, "list", ss.str());

  auto *e_solutions(d->NewElement("solutions"));
  e_summary->InsertEndChild(e_solutions);

  auto *e_runs(d->NewElement("runs"));
  e_solutions->InsertEndChild(e_runs);
  for (const auto &gr : good_runs)
    set_text(e_runs, "run", gr);
  set_text(e_solutions, "found", solutions);

  const auto avg_depth(solutions ? run_sum.last_imp / solutions : 0);
  set_text(e_solutions, "avg_depth", avg_depth);

  auto *e_other(d->NewElement("other"));
  e_summary->InsertEndChild(e_other);
  set_text(e_other,"evaluator", this->active_eva_->info());

  this->env_.xml(d);
}
#endif  // Include guard
