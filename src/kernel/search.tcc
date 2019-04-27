/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2019 EOS di Manlio Morini.
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
search<T, ES>::search(problem &p) : eva1_(nullptr), eva2_(nullptr),
                                    vs_(std::make_unique<as_is_validation>()),
                                    prob_(p)
{
  Ensures(debug());
}

template<class T, template<class> class ES>
bool search<T, ES>::can_validate() const
{
  return eva2_.get();
}

///
/// Calculates the fitness of the best individual so far.
///
/// \param[in] s summary of the evolution run just finished
/// \return      `s.best.score`
///
/// Specializations of this method can calculate further problem-specific
/// metrics regarding `s.best.solution` (via the `calculate_metrics_custom`
/// virtual function).
///
/// If a validation set / simulation is available, it's used for the
/// calculations.
///
template<class T, template<class> class ES>
model_measurements search<T, ES>::calculate_metrics(const summary<T> &s) const
{
  auto m(calculate_metrics_custom(s));

  if (can_validate())
  {
    assert(eva2_);
    m.fitness = (*eva2_)(s.best.solution);
  }
  else
  {
    assert(eva1_);
    m.fitness = (*eva1_)(s.best.solution);
  }

  return m;
}

///
/// For the base class this is just the identity function.
///
/// Derived classes could calculate additional problem-specific metrics.
///
template<class T, template<class> class ES>
model_measurements search<T, ES>::calculate_metrics_custom(
  const summary<T> &s) const
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
  const environment dflt(ES<T>::shape(environment().init()));
  const environment constrained(prob_.env);

  if (!constrained.mep.code_length)
    prob_.env.mep.code_length = dflt.mep.code_length;

  if (!constrained.mep.patch_length)
    prob_.env.mep.patch_length = 1 + prob_.sset.terminals(0) / 2;

  if (constrained.elitism == trilean::unknown)
    prob_.env.elitism = dflt.elitism;

  if (constrained.p_mutation < 0.0)
    prob_.env.p_mutation = dflt.p_mutation;

  if (constrained.p_cross < 0.0)
    prob_.env.p_cross = dflt.p_cross;

  if (!constrained.brood_recombination)
    prob_.env.brood_recombination = dflt.brood_recombination;

  if (!constrained.layers)
    prob_.env.layers = dflt.layers;

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

  if (!constrained.max_stuck_time.has_value())
    prob_.env.max_stuck_time = dflt.max_stuck_time;

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

  init();

  auto shake([this](unsigned g) { return vs_->shake(g); });

  summary<T> overall_summary;
  distribution<fitness_t> fd;

  unsigned best_run(0);
  std::vector<unsigned> good_runs;

  load();

  for (unsigned r(0); r < n; ++r)
  {
    vs_->init(r);
    auto run_summary(evolution<T, ES>(prob_, *eva1_).run(r, shake));
    vs_->close(r);

    // Possibly calculates additional metrics.
    run_summary.best.score = calculate_metrics(run_summary);

    after_evolution(&run_summary);

    print_resume(run_summary.best.score);

    if (r == 0
        || run_summary.best.score.fitness > overall_summary.best.score.fitness)
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

    assert(good_runs.empty()
           || std::find(std::begin(good_runs), std::end(good_runs), best_run)
              != std::end(good_runs));

    this->log_search(overall_summary, fd, good_runs, best_run, n);
  }

  save();

  return overall_summary;
}

///
/// Loads the saved evaluation cache from a file (if available).
///
/// \return `true` if the object is correctly loaded
///
template<class T, template<class> class ES>
bool search<T, ES>::load()
{
  if (prob_.env.misc.serialization_file.empty())
    return true;

  std::ifstream in(prob_.env.misc.serialization_file);
  if (!in)
    return false;

  if (prob_.env.cache_size)
  {
    if (!eva1_->load(in))
      return false;
    vitaINFO << "Loading cache";
  }

  return true;
}

///
/// \return `true` if the object was saved correctly
///
template<class T, template<class> class ES>
bool search<T, ES>::save() const
{
  if (prob_.env.misc.serialization_file.empty())
    return true;

  std::ofstream out(prob_.env.misc.serialization_file);
  if (!out)
    return false;

  if (prob_.env.cache_size)
  {
    if (!eva1_->save(out))
      return false;
    vitaINFO << "Saving cache";
  }

  return true;
}

///
/// \param[in] m metrics relative to the current run
///
template<class T, template<class> class ES>
void search<T, ES>::print_resume(const model_measurements &m) const
{
  const std::string s(can_validate() ? "Validation" : "Training");

  vitaINFO << s << " fitness: " << m.fitness;
}

///
/// Sets the main evaluator (used for training).
///
/// \tparam E an evaluator
///
/// \param[in] args arguments used to build the `E` evaluator
/// \return         a reference to the search class (used for method chaining)
///
/// \warning
/// We assume that the training evaluator could have a cache. This means that
/// changes in the training simulation / set should invalidate fitness values
/// stored in that cache.
///
template<class T, template<class> class ES>
template<class E, class... Args>
search<T, ES> &search<T, ES>::training_evaluator(Args && ...args)
{
  if (prob_.env.cache_size)
    eva1_ = std::make_unique<evaluator_proxy<T, E>>(
      E(std::forward<Args>(args)...), prob_.env.cache_size);
  else
    eva1_ = std::make_unique<E>(std::forward<Args>(args)...);

  return *this;
}

///
/// Sets the validation evaluator (used for validation).
///
/// \tparam E an evaluator
///
/// \param[in] args arguments used to build the `E` evaluator
/// \return         a reference to the search class (used for method chaining)
///
/// \warning
/// The validation evaluator cannot have a cache.
///
template<class T, template<class> class ES>
template<class E, class... Args>
search<T, ES> &search<T, ES>::validation_evaluator(Args && ...args)
{
  eva2_ = std::make_unique<E>(std::forward<Args>(args)...);
  return *this;
}

///
/// Sets the active validation strategy.
///
/// \param[in] v a new validation strategy
/// \return      a reference to the search class (used for method chaining)
///
template<class T, template<class> class ES>
template<class V, class... Args>
search<T, ES> &search<T, ES>::validation_strategy(Args && ...args)
{
  vs_ = std::make_unique<V>(std::forward<Args>(args)...);
  return *this;
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
void search<T, ES>::log_search(const summary<T> &run_sum,
                               const distribution<fitness_t> &fd,
                               const std::vector<unsigned> &good_runs,
                               unsigned best_run, unsigned runs) const
{
  if (prob_.env.stat.summary_file.empty())
    return;

  tinyxml2::XMLDocument d(false);

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
  set_text(e_other,"training_evaluator", eva1_->info());
  set_text(e_other,"validation_evaluator", eva2_->info());

  prob_.env.xml(&d);

  log_search_custom(&d, run_sum);

  const std::string f_sum(merge_path(prob_.env.stat.dir,
                                     prob_.env.stat.summary_file));
  d.SaveFile(f_sum.c_str());
}

#endif  // include guard
