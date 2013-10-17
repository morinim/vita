/**
 *
 *  \file search_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SEARCH_INL_H)
#define      SEARCH_INL_H

///
/// \param[in] prob a \c problem used for search initialization.
///
template<class T, template<class> class ES>
basic_search<T, ES>::basic_search(problem *const prob) : env_(prob->env),
                                                         prob_(prob)
{
  assert(prob->debug(true));

  assert(debug(true));
}

template<class T, template<class> class ES>
fitness_t basic_search<T, ES>::fitness(const T &ind)
{
  return (*prob_->get_evaluator())(ind);
}

///
/// \param[in] base \a individual we are examining to extract building blocks.
///
/// Adaptive Representation through Learning (ARL). The algorithm extract
/// common knowledge (building blocks) emerging during the evolutionary
/// process and acquires the necessary structure for solving the problem
/// (see ARL - Justinian P. Rosca and Dana H. Ballard).
///
template<class T, template<class> class ES>
void basic_search<T, ES>::arl(const T &base)
{
  const auto base_fit(fitness(base));
  if (!base_fit.isfinite())
    return;  // We need a finite fitness to search for an improvement

  // Logs ADFs
  const auto filename(env_.stat_dir + "/" + environment::arl_filename);
  std::ofstream log(filename.c_str(), std::ios_base::app);
  if (env_.stat_arl && log.good())
  {
    for (unsigned i(0); i < prob_->sset.adts(); ++i)
    {
      const symbol &f(*prob_->sset.get_adt(i));
      log << f.display() << ' ' << f.weight << std::endl;
    }
    log << std::endl;
  }

  const unsigned adf_args(0);
  auto blk_idx(base.blocks());
  for (const locus &l : blk_idx)
  {
    T candidate_block(base.get_block(l));

    // Building blocks must be simple.
    if (candidate_block.eff_size() <= 5 + adf_args)
    {
      // This is an approximation of the fitness due to the current block.
      // The idea is to see how the individual (base) would perform without
      // (base.destroy_block) the current block.
      // Useful blocks have delta values greater than 0.
      const auto delta(base_fit[0] -
                       fitness(base.destroy_block(l.index))[0]);

      // Semantic introns cannot be building blocks...
      // When delta is greater than 10% of the base fitness we have a
      // building block.
      if (std::isfinite(delta) && std::fabs(base_fit[0] / 10.0) < delta)
      {
        std::unique_ptr<symbol> p;
        if (adf_args)
        {
          std::vector<locus> replaced;
          T generalized(candidate_block.generalize(adf_args, &replaced));
          std::vector<category_t> categories(replaced.size());
          for (unsigned j(0); j < replaced.size(); ++j)
            categories[j] = replaced[j].category;

          p = make_unique<adf>(generalized, categories, 10);
        }
        else  // !adf_args
          p = make_unique<adt>(candidate_block, 100);

        if (env_.stat_arl && log.good())
        {
          log << p->display() << " (Base: " << base_fit
              << "  DF: " << delta
              << "  Weight: " << std::fabs(delta / base_fit[0]) * 100.0
              << "%)" << std::endl;
          candidate_block.list(log);
          log << std::endl;
        }

        prob_->sset.insert(std::move(p));
      }
    }
  }
}

///
/// \param[in] base \a team we are examining to extract building blocks.
///
/// Reapeatedly calls arl(const T &) for each member of the team.
///
template<class T, template<class> class ES>
void basic_search<T, ES>::arl(const basic_team<T> &base)
{
  for (const auto &ind : base)
    arl(ind);
}

///
/// \param[in] generation the generation that has been reached by the
///            evolution. The method uses this parameter to setup some
///            structures when generation == 0.
///
/// Dynamic Training Subset Selection for Supervised Learning in Genetic
/// Programming.
/// When using GP on a difficult problem, with a large set of training data,
/// a large population size is needed and a very large number of
/// function-trees evaluation must be carried out. DSS is a subset selection
/// method which uses the current run to select:
/// * firstly 'difficult' cases;
/// * secondly cases which have not been looked at for several generations.
///
template<class T, template<class> class ES>
void basic_search<T, ES>::dss(unsigned generation) const
{
  if (prob_->data())
  {
    data &d(*prob_->data());

    decltype(data::example::difficulty) weight_sum(0);
    auto weight([](const data::example &v) -> decltype(weight_sum)
                {
                  return v.difficulty + v.age * v.age * v.age;
                });

    d.dataset(data::training);
    d.slice(false);
    for (auto &i : d)
    {
      if (generation == 0)  // preliminary setup for generation 0
      {
        i.difficulty = 0;
        i.age        = 1;
      }
      else
        ++i.age;

      weight_sum += weight(i);
    }

    // Select a subset of the training examples.
    // Training examples, contained in d, are partitioned into two subsets
    // by multiple swaps (first subset: [0, count[,  second subset:
    // [count, d.size()[).
    // Note that the actual size of the selected subset (count) is not fixed
    // and, in fact, it averages slightly above target_size (Gathercole and
    // Ross felt that this might improve performance).
    const double ratio(std::min(0.6, 0.2 + 100.0 / (d.size() + 100.0)));
    assert(0.2 <= ratio && ratio <= 0.6);
    const size_t target_size(d.size() * ratio);
    assert(target_size && target_size <= d.size());

    data::iterator base(d.begin());
    unsigned count(0);
    for (auto i(d.begin()); i != d.end(); ++i)
    {
      const auto prob(
        std::min(static_cast<double>(weight(*i)) * target_size / weight_sum,
                 1.0));

      if (random::boolean(prob))
      {
        std::iter_swap(base, i);
        ++base;
        ++count;
      }
    }

    d.slice(std::max(count, 10u));
    prob_->get_evaluator()->clear(evaluator::all);

    // Selected training examples have their difficulties and ages reset.
    for (auto &i : d)
    {
      i.difficulty = 0;
      i.age        = 1;
    }
  }
}

///
/// \param[in] s an up to date run summary.
/// \return \c true when a run should be interrupted.
///
template<class T, template<class> class ES>
bool basic_search<T, ES>::stop_condition(const summary<T> &s) const
{
  assert(env_.generations);

  if (s.gen > env_.generations)
    return true;

  // We use an accelerated stop condition when all the individuals have
  // the same fitness and after env_.g_without_improvement generations the
  // situation isn't changed.
  assert(env_.g_without_improvement);

  if (*env_.g_without_improvement &&
      (s.gen - s.last_imp > *env_.g_without_improvement &&
       s.az.fit_dist().variance.issmall()))
    return true;

  return false;
}

///
/// \brief Tries to tune search parameters for the current problem.
///
/// Parameter tuning is a typical approach to algorithm design. Such tuning
/// is done by experimenting with different values and selecting the ones
/// that give the best results on the test problems at hand.
///
/// However, the number of possible parameters and their different values
/// means that this is a very complex and time-consuming task; it is
/// something we do not want users to worry about (power users can force many
/// parameters, but our idea is "simple by default").
///
/// So if user sets an environment parameter he will force the search class
/// to use it as is. Otherwise this function will try to guess a good
/// starting point and changes its hint after every run. The code is a mix of
/// black magic, experience, common logic and randomness but it seems
/// reasonable.
///
/// \note
/// It has been formally proven, in the No-Free-Lunch theorem, that it is
/// impossible to tune a search algorithm such that it will have optimal
/// settings for all possible problems, but parameters can be properly
/// set for a given problem.
///
/// \see
/// * "Parameter Setting in Evolutionary Algorithms" (F.G. Lobo, C.F. Lima,
///   Z. Michalewicz) - Springer;
/// * "Genetic Programming - An Introduction" (Banzhaf, Nordin, Keller,
///   Francone).
///
template<class T, template<class> class ES>
void basic_search<T, ES>::tune_parameters()
{
  const environment dflt(true);
  const environment &constrained(prob_->env);

  const data *const dt(prob_->data());

  if (constrained.code_length == 0)
    env_.code_length = dflt.code_length;

  if (constrained.patch_length == 0)
    env_.patch_length = 1 + env_.code_length / 3;

  if (boost::indeterminate(constrained.elitism))
    env_.elitism = dflt.elitism;

  if (constrained.p_mutation >= 0.0)
    env_.p_mutation = dflt.p_mutation;

  if (!constrained.p_cross)
    env_.p_cross = *dflt.p_cross;

  if (!constrained.brood_recombination)
    env_.brood_recombination = *dflt.brood_recombination;

  // With a small number of training case:
  // * we need every training case;
  // * DSS speed up isn't so sensible;
  // BUT
  // * DSS can help against overfitting.
  if (boost::indeterminate(constrained.dss))
  {
    env_.dss = dt && dt->size() > 400;

    if (env_.verbosity >= 2)
      std::cout << k_s_info << " DSS set to " << env_.dss << std::endl;
  }

  if (!constrained.layers)
  {
    if (dt && dt->size() > 8)
      env_.layers = std::log(dt->size());
    else
      env_.layers = dflt.layers;

    if (env_.verbosity >= 2)
      std::cout << k_s_info << " Number of layers set to " << env_.layers
                << std::endl;
  }

  // A larger number of training cases requires an increase in the population
  // size (e.g. in "Genetic Programming - An Introduction" Banzhaf, Nordin,
  // Keller and Francone suggest 10 - 1000 individuals for smaller problems;
  // between 1000 and 10000 individuals for complex problem (more than 200
  // fitness cases).
  //
  // We choosed a strictly increasing function to link training set size
  // and population size.
  if (!constrained.individuals)
  {
    if (dt && dt->size() > 8)
    {
      env_.individuals = 2 * std::pow((std::log2(dt->size())), 3) / env_.layers;

      if (env_.individuals < 4)
        env_.individuals = 4;
    }
    else
      env_.individuals = dflt.individuals;

    if (env_.verbosity >= 2)
      std::cout << k_s_info << " Population size set to " << env_.individuals
                << std::endl;
  }

  // Note that this setting, once set, will not be changed.
  if (!constrained.validation_ratio && !env_.validation_ratio)
  {
    if (dt)
    {
      if (dt->size() * (*dflt.validation_ratio) < 100.0)
        env_.validation_ratio = 0.0;
      else
        env_.validation_ratio = *dflt.validation_ratio;
    }
    else
      env_.validation_ratio = *dflt.validation_ratio;

    if (env_.verbosity >= 2)
      std::cout << k_s_info << " Validation ratio set to "
                << 100.0 * (*env_.validation_ratio) << '%' << std::endl;
  }

  if (!constrained.tournament_size)
    env_.tournament_size = dflt.tournament_size;

  if (!constrained.mate_zone)
    env_.mate_zone = *dflt.mate_zone;

  if (!constrained.generations)
    env_.generations = dflt.generations;

  if (!constrained.g_without_improvement)
    env_.g_without_improvement = *dflt.g_without_improvement;

  if (boost::indeterminate(constrained.arl))
    env_.arl = dflt.arl;

  assert(env_.debug(true, true));
}

///
/// param[in] ind an individual.
/// \return the accuracy of \a ind.
///
/// \note
/// If env_.a_threashold < 0.0 (undefined) then this method will skip
/// accuracy calculation and return will return a negative value.
///
/// \warning
/// This method can be very time consuming.
///
template<class T, template<class> class ES>
double basic_search<T, ES>::accuracy(const T &ind) const
{
  if (env_.a_threashold < 0.0)
    return env_.a_threashold;

  return prob_->get_evaluator()->accuracy(ind);
}

///
/// \param[in] validation is it a validation or training resume?
/// \param[in] fitness fitness reached in the current run.
/// \param[in] accuracy accuracy reached in the current run.
///
template<class T, template<class> class ES>
void basic_search<T, ES>::print_resume(bool validation,
                                       const fitness_t &fitness,
                                       double accuracy) const
{
  if (env_.verbosity >= 2)
  {
    const std::string ds(validation ? " Validation" : " Training");

    std::cout << k_s_info << ds << " fitness: " << fitness << std::endl;
    if (env_.a_threashold >= 0.0)
      std::cout << k_s_info << ds << " accuracy: " << 100.0 * accuracy
                << '%';

    std::cout << std::endl << std::endl;
  }
}

///
/// \param[in] n number of runs.
/// \return best individual found.
///
template<class T, template<class> class ES>
T basic_search<T, ES>::run(unsigned n)
{
  assert(env_.f_threashold != vita::fitness_t() || env_.a_threashold > 0.0);

  summary<T> overall_summary;
  distribution<fitness_t> fd;

  double best_accuracy(-1.0);
  unsigned best_run(0);

  std::list<unsigned> good_runs;

  tune_parameters();

  // For std::placeholders and std::bind see:
  //  <http://en.cppreference.com/w/cpp/utility/functional/placeholders>
  std::function<void (unsigned)> shake_data;
  if (env_.dss)
    shake_data = std::bind(&search::dss, this, std::placeholders::_1);

  std::function<bool (const summary<T> &)> stop;
  if (*env_.g_without_improvement > 0)
    stop = std::bind(&search::stop_condition, this, std::placeholders::_1);

  const bool validation(env_.validation_ratio &&
                        *env_.validation_ratio > 0.0);
  if (validation)
    prob_->data()->divide(*env_.validation_ratio);

  for (unsigned run(0); run < n; ++run)
  {
    evolution<ES<T>> evo(env_, prob_->sset, *prob_->get_evaluator(), stop,
                         shake_data);
    summary<T> s(evo.run(run));

    // Depending on validation, this can be the training fitness or the
    // validation fitness for the current run.
    fitness_t run_fitness;

    // Depending on validation, this can be the training accuracy or the
    // validation accuracy for the current run.
    double run_accuracy(-1.0);

    if (validation)
    {
      const data::dataset_t backup(prob_->data()->dataset());

      prob_->data()->dataset(data::validation);
      prob_->get_evaluator()->clear(s.best->ind);

      run_fitness = fitness(s.best->ind);
      run_accuracy = accuracy(s.best->ind);

      prob_->data()->dataset(backup);
      prob_->get_evaluator()->clear(s.best->ind);
    }
    else  // not using a validation set
    {
      // If shake_data is true, the values calculated during the evolution
      // refer to a subset of the available training set. Since we need an
      // overall fitness for comparison, a new calculation has to be
      // performed.
      if (shake_data)
      {
        prob_->data()->dataset(data::training);
        prob_->data()->slice(false);
        prob_->get_evaluator()->clear(s.best->ind);

        run_fitness = fitness(s.best->ind);
      }
      else
        run_fitness = s.best->fitness;

      run_accuracy = accuracy(s.best->ind);
    }

    print_resume(validation, run_fitness, run_accuracy);

    if (run == 0 || run_fitness > overall_summary.best->fitness)
    {
      overall_summary.best = {s.best->ind, run_fitness};
      best_accuracy = run_accuracy;
      best_run = run;
    }

    // We use accuracy or fitness (or both) to identify successful runs.
    const bool solution_found(run_fitness.dominating(env_.f_threashold) &&
                              run_accuracy >= env_.a_threashold);

    if (solution_found)
    {
      overall_summary.last_imp += s.last_imp;

      good_runs.push_back(run);
    }

    if (run_fitness.isfinite())
      fd.add(run_fitness);

    overall_summary.speed = overall_summary.speed +
      (s.speed - overall_summary.speed) / (run + 1);

    if (env_.arl && good_runs.front() == run)
    {
      prob_->sset.reset_adf_weights();
      arl(s.best->ind);
    }

    assert(good_runs.empty() ||
           std::find(good_runs.begin(), good_runs.end(), best_run) !=
           good_runs.end());
    log(overall_summary, fd, good_runs, best_run, best_accuracy, n);
  }

  return overall_summary.best->ind;
}

///
/// \param[in] run_sum summary information regarding the search.
/// \param[in] fd statistics about population fitness.
/// \param[in] good_runs list of the best runs of the search.
/// \param[in] best_run best overall run.
/// \param[in] best_accuracy accuracy of the best individual (if available).
/// \param[in] runs number of runs performed.
/// \return \c true if the write operation succeed.
///
/// Writes end-of-run logs (run summary, results for test...).
///
template<class T, template<class> class ES>
void basic_search<T, ES>::log(const summary<T> &run_sum,
                              const distribution<fitness_t> &fd,
                              const std::list<unsigned> &good_runs,
                              unsigned best_run, double best_accuracy,
                              unsigned runs)
{
  // Summary logging.
  if (env_.stat_summary)
  {
    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best->ind.list(best_list);
    run_sum.best->ind.tree(best_tree);
    run_sum.best->ind.graphviz(best_graph);

    const std::string path("vita.");
    const std::string summary(path + "summary.");

    const unsigned solutions(good_runs.size());

    boost::property_tree::ptree pt;
    pt.put(summary + "success_rate", runs ?
           static_cast<double>(solutions) / static_cast<double>(runs) : 0);
    pt.put(summary + "speed", run_sum.speed);
    pt.put(summary + "mean_fitness", fd.mean);
    pt.put(summary + "standard_deviation", fd.standard_deviation());

    pt.put(summary + "best.fitness", run_sum.best->fitness);
    pt.put(summary + "best.accuracy", best_accuracy);
    pt.put(summary + "best.run", best_run);
    pt.put(summary + "best.individual.tree", best_tree.str());
    pt.put(summary + "best.individual.list", best_list.str());
    pt.put(summary + "best.individual.graph", best_graph.str());

    for (const auto &p : good_runs)
      pt.add(summary + "solutions.runs.run", p);
    pt.put(summary + "solutions.found", solutions);
    pt.put(summary + "solutions.avg_depth",
           solutions ? run_sum.last_imp / solutions : 0);

    pt.put(summary + "other.evaluator", prob_->get_evaluator()->info());

    const std::string f_sum(env_.stat_dir + "/" + environment::sum_filename);

    env_.log(&pt, path);

    using namespace boost::property_tree;
    write_xml(f_sum, pt, std::locale(), xml_writer_make_settings(' ', 2));
  }

  // Test set results logging.
  vita::data &data(*prob_->data());
  if (data.size(data::test))
  {
    const data::dataset_t backup(data.dataset());
    data.dataset(data::test);

    std::unique_ptr<lambda_f> lambda(prob_->lambdify(run_sum.best->ind));

    std::ofstream tf(env_.stat_dir + "/" + environment::tst_filename);
    for (const auto &example : data)
      tf << lambda->name((*lambda)(example)) << std::endl;

    data.dataset(backup);
  }
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T, template<class> class ES>
bool basic_search<T, ES>::debug(bool verbose) const
{
  return prob_->debug(verbose);
}
#endif  // SEARCH_INL_H
