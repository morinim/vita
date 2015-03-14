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
  const environment dflt(true);
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

  assert(this->env_.debug(true, true));
}

///
/// \param[in] n number of runs.
/// \return a summary of the search.
///
template<class T, template<class> class ES, class F>
summary<T> ga_search<T, ES, F>::run_nvi(unsigned n)
{
  summary<T> overall_summary;
  distribution<fitness_t> fd;

  unsigned best_run(0);

  std::vector<unsigned> good_runs;

  tune_parameters_nvi();

  for (unsigned r(0); r < n; ++r)
  {
    auto &eval(*this->active_eva_);  // just a short-cut
    evolution<T, ES> evo(this->env_, this->prob_.sset, eval, nullptr, nullptr);
    summary<T> s(evo.run(r));

    // The training fitness for the current run.
    fitness_t run_fitness;

    run_fitness = s.best.fitness;

    print_resume(run_fitness);

    if (r == 0 || run_fitness > overall_summary.best.fitness)
    {
      overall_summary.best = {s.best.solution, run_fitness, -1.0};
      best_run = r;
    }

    // We use fitness to identify successful runs.
    const bool solution_found(run_fitness.dominating(
                                this->env_.threshold.fitness));

    if (solution_found)
    {
      overall_summary.last_imp += s.last_imp;

      good_runs.push_back(r);
    }

    if (isfinite(run_fitness))
      fd.add(run_fitness);

    overall_summary.elapsed += s.elapsed;

    assert(good_runs.empty() ||
           std::find(good_runs.begin(), good_runs.end(), best_run) !=
           good_runs.end());
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
  if (this->env_.verbosity >= 2)
  {
    std::cout << k_s_info << " Fitness: " << fit
              << "\n\n\n";
  }
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
void ga_search<T, ES, F>::log(const summary<T> &run_sum,
                              const distribution<fitness_t> &fd,
                              const std::vector<unsigned> &good_runs,
                              unsigned best_run, unsigned runs)
{
  // Summary logging.
  if (this->env_.stat_summary)
  {
    std::ostringstream best_list;
    run_sum.best.solution.list(best_list);

    const std::string path("vita.");
    const std::string summary(path + "summary.");

    const auto solutions(static_cast<unsigned>(good_runs.size()));

    boost::property_tree::ptree pt;
    pt.put(summary + "success_rate", runs ?
           static_cast<double>(solutions) / static_cast<double>(runs) : 0);
    pt.put(summary + "elapsed_time", run_sum.elapsed);
    pt.put(summary + "mean_fitness", fd.mean());
    pt.put(summary + "standard_deviation", fd.standard_deviation());

    pt.put(summary + "best.fitness", run_sum.best.fitness);
    pt.put(summary + "best.run", best_run);
    pt.put(summary + "best.solution.list", best_list.str());

    for (const auto &p : good_runs)
      pt.add(summary + "solutions.runs.run", p);
    pt.put(summary + "solutions.found", solutions);
    pt.put(summary + "solutions.avg_depth",
           solutions ? run_sum.last_imp / solutions : 0);

    pt.put(summary + "other.evaluator", this->active_eva_->info());

    const std::string f_sum(this->env_.stat_dir + "/" +
                            environment::sum_filename);

    this->env_.log(&pt, path);

    using namespace boost::property_tree;
#if BOOST_VERSION >= 105600
    write_xml(f_sum, pt, std::locale(),
              xml_writer_make_settings<std::string>(' ', 2));
#else
    write_xml(f_sum, pt, std::locale(),
              xml_writer_make_settings(' ', 2));
#endif
  }
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T, template<class> class ES, class F>
bool ga_search<T, ES, F>::debug_nvi(bool) const
{
  return true;
}
#endif  // Include guard
