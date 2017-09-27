/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
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
///              the lifetime of `this` class
///
template<class T, template<class> class ES>
search<T, ES>::search(problem &p) : active_eva_(nullptr),
                                    vs_(std::make_unique<as_is_validation>()),
                                    prob_(p)
{
  Ensures(debug());
}

///
/// Calculates problem-specific metrics regarding the solution found.
///
/// \param[in] s summary of the evolution run just finished
/// \return      the fitness of `s.best.solution` (other record of
///              model_measurements are unmodified).
///
/// Specializations of this method can calculate further problem specific
/// metrics regarding `s.best.solution`.
///
template<class T, template<class> class ES>
model_measurements search<T, ES>::calculate_metrics(const summary<T> &s) const
{
  return s.best.score;
}

///
/// Tries to tune search parameters for the current problem.
///
template<class T, template<class> class ES>
void search<T, ES>::tune_parameters()
{
  // The `shape` function modifies the default parameters with
  // strategy-specific values.
  const environment dflt(ES<T>::shape(environment(initialization::standard)));
  const environment constrained(prob_.env);

  if (!constrained.code_length)
    prob_.env.code_length = dflt.code_length;

  if (!constrained.patch_length)
    prob_.env.patch_length = 1 + prob_.sset.terminals(0) / 2;

  if (constrained.elitism == trilean::unknown)
    prob_.env.elitism = dflt.elitism;

  if (constrained.p_mutation < 0.0)
    prob_.env.p_mutation = dflt.p_mutation;

  if (constrained.p_cross < 0.0)
    prob_.env.p_cross = dflt.p_cross;

  if (!constrained.brood_recombination)
    prob_.env.brood_recombination = dflt.brood_recombination;

  if (constrained.dss == trilean::unknown)
    prob_.env.dss = trilean::no;

  if (!constrained.layers)
    prob_.env.layers = dflt.layers;

  if (constrained.validation_percentage == 100)
    prob_.env.validation_percentage = dflt.validation_percentage;

  if (!constrained.individuals)
    prob_.env.individuals = dflt.individuals;

  if (!constrained.min_individuals)
    prob_.env.min_individuals = dflt.min_individuals;

  if (!constrained.tournament_size)
    prob_.env.tournament_size = dflt.tournament_size;

  if (!constrained.mate_zone)
    prob_.env.mate_zone = dflt.mate_zone;

  if (!constrained.generations)
    prob_.env.generations = dflt.generations;

  if (!constrained.g_without_improvement)
    prob_.env.g_without_improvement = dflt.g_without_improvement;

  if (constrained.arl == trilean::unknown)
    prob_.env.arl = dflt.arl;

  Ensures(prob_.env.debug(true));
}

///
/// \param[in] n number of runs
/// \return      a summary of the search
///
template<class T, template<class> class ES>
summary<T> search<T, ES>::run(unsigned n)
{
  tune_parameters();

  preliminary_setup();

  vs_->preliminary_setup();
  auto shake([this](unsigned g) { return vs_->shake(g); });

  summary<T> overall_summary;
  distribution<fitness_t> fd;

  unsigned best_run(0);
  std::vector<unsigned> good_runs;

  load();

  for (unsigned r(0); r < n; ++r)
  {
    auto run_summary(evolution<T, ES>(prob_, *active_eva_).run(r, shake));

    // If a validation test is available the performance of the best trained
    // individual is recalculated.
    if (prob_.data() && prob_.data()->has(data::validation))
    {
      prob_.data()->select(data::validation);
      active_eva_->clear(run_summary.best.solution);

      run_summary.best.score.fitness=(*active_eva_)(run_summary.best.solution);
      run_summary.best.score = calculate_metrics(run_summary);

      prob_.data()->select(data::training);
      active_eva_->clear(run_summary.best.solution);
    }
    else
      run_summary.best.score = calculate_metrics(run_summary);

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
                              this->prob_.env.threshold);

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

  save();

  return overall_summary;
}

///
/// \return `true` if the object is correctly loaded
///
template<class T, template<class> class ES>
bool search<T, ES>::load()
{
  if (prob_.env.misc.serialization_name.empty())
    return true;

  std::ifstream in(prob_.env.misc.serialization_name.c_str());
  if (!in)
    return false;

  if (prob_.env.cache_size)
  {
    if (!active_eva_->load(in))
      return false;
    print.info("Loading cache");
  }

  return true;
}

///
/// \return `true` if the object was saved correctly
///
template<class T, template<class> class ES>
bool search<T, ES>::save() const
{
  if (prob_.env.misc.serialization_name.empty())
    return true;

  std::ofstream out(prob_.env.misc.serialization_name.c_str());
  if (!out)
    return false;

  if (prob_.env.cache_size)
  {
    if (!active_eva_->save(out))
      return false;
    print.info("Saving cache");
  }

  return true;
}

///
/// \param[in] m metrics relative to the current run
///
template<class T, template<class> class ES>
void search<T, ES>::print_resume(const model_measurements &m) const
{
  const std::string s(prob_.data() &&
                      prob_.data()->has(data::validation) ? "Validation "
                                                          : "Training ");

  print.info(s, "fitness: ", m.fitness);
}

///
/// Sets the active evaluator.
///
/// \tparam E an evaluator
///
/// \param[in] args arguments used to build the `E` evaluator
///
template<class T, template<class> class ES>
template<class E, class ...Args>
void search<T, ES>::set_evaluator(Args &&... args)
{
  if (prob_.env.cache_size)
    active_eva_ = std::make_unique<evaluator_proxy<T, E>>(
      E(std::forward<Args>(args)...), prob_.env.cache_size);
  else
    active_eva_ = std::make_unique<E>(std::forward<Args>(args)...);
}

///
/// \param[in] v a new validation strategy
///
template<class T, template<class> class ES>
void search<T, ES>::set_validator(std::unique_ptr<validation_strategy> v)
{
  Expects(v);

  vs_ = std::move(v);
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T, template<class> class ES>
bool search<T, ES>::debug() const
{
  return prob_.debug();
}

///
/// Writes end-of-run logs (run summary, results for test...).
///
/// \param[in] run_sum       summary information regarding the search
/// \param[in] fd statistics about population fitness
/// \param[in] good_runs     list of the best runs of the search
/// \param[in] best_run      best overall run
/// \param[in] runs          number of runs performed
/// \return                  `true` if the write operation succeed
///
template<class T, template<class> class ES>
void search<T, ES>::log(const summary<T> &run_sum,
                        const distribution<fitness_t> &fd,
                        const std::vector<unsigned> &good_runs,
                        unsigned best_run, unsigned runs) const
{
  // Summary logging.
  if (!prob_.env.stat.summary)
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
  set_text(e_summary, "elapsed_time", run_sum.elapsed.count());
  set_text(e_summary, "mean_fitness", fd.mean());
  set_text(e_summary, "standard_deviation", fd.standard_deviation());

  auto *e_best(d.NewElement("best"));
  e_summary->InsertEndChild(e_best);
  set_text(e_best, "fitness", run_sum.best.score.fitness);
  set_text(e_best, "run", best_run);

  std::ostringstream ss;
  ss << out::print_format(prob_.env.stat.ind_format) << run_sum.best.solution;
  set_text(e_best, "code", ss.str());

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

  prob_.env.xml(&d);

  log_nvi(&d, run_sum);

  const std::string f_sum(prob_.env.stat.dir + "/" + prob_.env.stat.sum_name);
  d.SaveFile(f_sum.c_str());
}

#endif  // include guard
