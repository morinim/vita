/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2021 EOS di Manlio Morini.
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
                                    prob_(p), after_generation_callback_()
{
  Ensures(is_valid());
}

template<class T, template<class> class ES>
search<T, ES> &search<T, ES>::after_generation(
  typename evolution<T, ES>::after_generation_callback_t f)
{
  after_generation_callback_ = std::move(f);
  return *this;
}

template<class T, template<class> class ES>
bool search<T, ES>::can_validate() const
{
  return eva2_.get();
}

///
/// Calculates and stores the fitness of the best individual so far.
///
/// \param[in] s summary of the evolution run just finished
///
/// Specializations of this method can calculate further / distinct
/// problem-specific metrics regarding the candidate solution.
///
/// If a validation set / simulation is available, it's used for the
/// calculations.
///
template<class T, template<class> class ES>
void search<T, ES>::calculate_metrics(summary<T> *s) const
{
  auto &best(s->best);

  if (can_validate())
  {
    assert(eva2_);
    best.score.fitness = (*eva2_)(best.solution);
  }
  else
  {
    assert(eva1_);
    best.score.fitness = (*eva1_)(best.solution);
  }

  // We use accuracy or fitness (or both) to identify successful runs.
  best.score.is_solution = (best.score >= this->prob_.env.threshold);
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

  Ensures(prob_.env.is_valid(true));
}

///
/// Performs basic initialization before the search.
///
/// The default behaviour involve:
/// - tuning of the search parameters;
/// - possibly loading cached value for the training evaluator.
///
/// \remark
/// Called at the beginning of the first run (i.e. only one time even for a
/// multiple-run search).
///
template<class T, template<class> class ES>
void search<T, ES>::init()
{
  tune_parameters();

  load();
}

///
/// Performs closing actions at the end of the search.
///
/// The default behaviour involve (possibly) caching values of the training
/// evaluator.
///
/// \remark
/// Called at the beginning of the first run (i.e. only one time even for a
/// multiple-run search).
///
template<class T, template<class> class ES>
void search<T, ES>::close()
{
  save();
}

///
/// Performs after evolution tasks.
///
/// The default act is to print the result of the evolutionary run. Derived
/// classes can change / integrate the base behaviour.
///
/// \remark Called at the end of each run.
///
template<class T, template<class> class ES>
void search<T, ES>::after_evolution(const summary<T> &s)
{
  print_resume(s.best.score);
}

///
/// \param[in] n number of runs
/// \return      a summary of the search
///
template<class T, template<class> class ES>
summary<T> search<T, ES>::run(unsigned n)
{
  init();

  auto shake([this](unsigned g) { return vs_->shake(g); });
  search_stats<T> stats;

  for (unsigned r(0); r < n; ++r)
  {
    vs_->init(r);
    auto run_summary(evolution<T, ES>(prob_, *eva1_)
                     .after_generation(after_generation_callback_)
                     .run(r, shake));
    vs_->close(r);

    // Possibly calculates additional metrics.
    calculate_metrics(&run_summary);

    after_evolution(run_summary);

    stats.update(run_summary);
    log_stats(stats);
  }

  close();

  return stats.overall;
}

template<class T>
void search_stats<T>::update(const summary<T> &r)
{
  if (runs == 0 || r.best.score.fitness > overall.best.score.fitness)
  {
    overall.best = r.best;
    best_run = runs;
  }

  if (r.best.score.is_solution)
  {
    overall.last_imp += r.last_imp;
    good_runs.insert(good_runs.end(), runs);
  }

  if (isfinite(r.best.score.fitness))
    fd.add(r.best.score.fitness);

  overall.elapsed += r.elapsed;
  overall.gen += r.gen;

  ++runs;

  Ensures(good_runs.empty() || good_runs.count(best_run));
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
/// Prints a resume of the evolutionary run.
///
/// \param[in] m metrics relative to the current run
///
/// Derived classes can add further specific information.
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
bool search<T, ES>::is_valid() const
{
  return true;
}

///
/// Writes end-of-run logs (run summary, results for test...).
///
/// \param[in]  stats mixed statistics about the search performed so far
/// \param[out] d     output file (XML)
///
template<class T, template<class> class ES>
void search<T, ES>::log_stats(const search_stats<T> &stats,
                              tinyxml2::XMLDocument *d) const
{
  Expects(d);

  if (prob_.env.stat.summary_file.empty())
    return;

  auto *root(d->NewElement("vita"));
  d->InsertFirstChild(root);

  auto *e_summary(d->NewElement("summary"));
  root->InsertEndChild(e_summary);

  const auto solutions(stats.good_runs.size());
  const auto success_rate(
    stats.runs ? static_cast<double>(solutions)
                 / static_cast<double>(stats.runs)
               : 0);

  set_text(e_summary, "success_rate", success_rate);
  set_text(e_summary, "elapsed_time", stats.overall.elapsed.count());
  set_text(e_summary, "mean_fitness", stats.fd.mean());
  set_text(e_summary, "standard_deviation", stats.fd.standard_deviation());

  auto *e_best(d->NewElement("best"));
  e_summary->InsertEndChild(e_best);
  set_text(e_best, "fitness", stats.overall.best.score.fitness);
  set_text(e_best, "run", stats.best_run);

  std::ostringstream ss;
  ss << out::print_format(prob_.env.stat.ind_format)
     << stats.overall.best.solution;
  set_text(e_best, "code", ss.str());

  auto *e_solutions(d->NewElement("solutions"));
  e_summary->InsertEndChild(e_solutions);

  auto *e_runs(d->NewElement("runs"));
  e_solutions->InsertEndChild(e_runs);
  for (const auto &gr : stats.good_runs)
    set_text(e_runs, "run", gr);
  set_text(e_solutions, "found", solutions);

  const auto avg_depth(solutions ? stats.overall.last_imp / solutions
                                 : 0);
  set_text(e_solutions, "avg_depth", avg_depth);

  auto *e_other(d->NewElement("other"));
  e_summary->InsertEndChild(e_other);

  prob_.env.xml(d);
}

template<class T, template<class> class ES>
void search<T, ES>::log_stats(const search_stats<T> &stats) const
{
  tinyxml2::XMLDocument d(false);

  log_stats(stats, &d);

  const auto f_sum(prob_.env.stat.dir / prob_.env.stat.summary_file);
  d.SaveFile(f_sum.string().c_str());
}

#endif  // include guard
