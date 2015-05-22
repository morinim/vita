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

#if !defined(VITA_SRC_SEARCH_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_SRC_SEARCH_TCC)
#define      VITA_SRC_SEARCH_TCC

constexpr std::underlying_type<metric_flags>::type operator&(metric_flags f1,
                                                             metric_flags f2)
{
  using type = std::underlying_type<metric_flags>::type;

  return static_cast<type>(f1) & static_cast<type>(f2);
}

///
/// \param[in] p the problem we're working on. The lifetime of `p` must exceed
///              the lifetime of `this` class.
/// \param[in] m a bit field used to specify matrics we have to calculate while
///              searching.
///
template<class T, template<class> class ES>
src_search<T, ES>::src_search(src_problem &p, metric_flags m)
  : search<T, ES>(p),
    p_symre(evaluator_id::rmae), p_class(evaluator_id::gaussian), metrics(m)
{
  assert(p.debug(true));

  if (p.data()->size() && !this->active_eva_)
    set_evaluator(p.classification() ? p_class : p_symre);

  assert(this->debug(true));
}

///
/// \param[in] ind an individual.
/// \param[out] res metrics regarding `ind`.
///
/// Accuracy calculation is performed if AT LEAST ONE of the following
/// conditions is satisfied:
///
/// * the accuracy threshold is defined (`env_.threshold.accuracy > 0.0`);
/// * we explicitly asked for accuracy calculation (see the `src_search`
///   constructor).
///
/// Otherwise the function will skip accuracy calculation, returning a negative
/// value.
///
/// \warning Could be very time consuming.
///
template<class T, template<class> class ES>
void src_search<T, ES>::calculate_metrics(
  const T &ind, typename summary<T>::measurements *out) const
{
  if (metrics & metric_flags::accuracy || this->env_.threshold.accuracy > 0.0)
  {
    const auto model(this->lambdify(ind));
    out->accuracy = model->measure(accuracy_metric<T>(), *this->prob_.data());
  }
}

///
/// \param[in] base individual we are examining to extract building blocks.
///
/// Adaptive Representation through Learning (ARL). The algorithm extract
/// common knowledge (building blocks) emerging during the evolutionary
/// process and acquires the necessary structure for solving the problem
/// (see ARL - Justinian P. Rosca and Dana H. Ballard).
///
/// \note
/// No partial specialization for member functions of class templates is
/// allowed but we need it for this method (we need partial specialization on
/// T).
/// The tipical work around is to introduce overloaded functions inside the
/// class (this has the benefit that they have the same access to member
/// variables, functions...).
///
template<class T, template<class> class ES>
template<class U>
void src_search<T, ES>::arl(const U &base)
{
  const auto base_fit(this->fitness(base));
  if (!isfinite(base_fit))
    return;  // We need a finite fitness to search for an improvement

  // Logs ADFs
  const auto filename(this->env_.stat.dir + "/" + this->env_.stat.arl_name);
  std::ofstream adf_log(filename, std::ios_base::app);
  if (this->env_.stat.arl && adf_log.good())
  {
    const auto adts(this->prob_.env.sset->adts());
    for (auto i(decltype(adts){0}); i < adts; ++i)
    {
      const symbol &f(*this->prob_.env.sset->get_adt(i));
      adf_log << f.display() << ' ' << f.weight << '\n';
    }
    adf_log << '\n';
  }

  const unsigned adf_args(0);
  auto blk_idx(base.blocks());
  for (const locus &l : blk_idx)
  {
    auto candidate_block(base.get_block(l));

    // Building blocks must be simple.
    if (candidate_block.eff_size() <= 5 + adf_args)
    {
      // This is an approximation of the fitness due to the current block.
      // The idea is to see how the individual (base) would perform without
      // (base.destroy_block) the current block.
      // Useful blocks have delta values greater than 0.
      const auto delta(base_fit[0] -
                       this->fitness(base.destroy_block(l.index))[0]);

      // Semantic introns cannot be building blocks...
      // When delta is greater than 10% of the base fitness we have a
      // building block.
      if (std::isfinite(delta) && std::fabs(base_fit[0] / 10.0) < delta)
      {
        std::unique_ptr<symbol> p;
        if (adf_args)
        {
          auto generalized(candidate_block.generalize(adf_args));
          cvect categories(generalized.second.size());

          for (const auto &replaced : generalized.second)
            categories.push_back(replaced.category);

          p = vita::make_unique<adf>(generalized.first, categories, 10u);
        }
        else  // !adf_args
          p = vita::make_unique<adt>(candidate_block, 100u);

        if (this->env_.stat.arl && adf_log.good())
        {
          adf_log << p->display() << " (Base: " << base_fit
                  << "  DF: " << delta
                  << "  Weight: " << std::fabs(delta / base_fit[0]) * 100.0
                  << "%)\n";
          candidate_block.list(adf_log);
          adf_log << '\n';
        }

        this->prob_.env.sset->insert(std::move(p));
      }
    }
  }
}

///
/// \param[in] base a team we are examining to extract building blocks.
///
/// Repeatedly calls arl(const U &) for each member of the team.
///
template<class T, template<class> class ES>
template<class U>
void src_search<T, ES>::arl(const team<U> &)
{

}

///
/// \brief Tries to tune search parameters for the current problem
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
void src_search<T, ES>::tune_parameters_nvi()
{
  const environment dflt(nullptr, true);
  const environment &constrained(this->prob_.env);

  const auto d_size(this->prob_.data() ? this->prob_.data()->size() : 0);

  if (constrained.code_length == 0)
    this->env_.code_length = dflt.code_length;

  if (constrained.patch_length == 0)
    this->env_.patch_length = 1 + this->env_.code_length / 3;

  if (constrained.elitism == trilean::unknown)
    this->env_.elitism = dflt.elitism;

  if (constrained.p_mutation < 0.0)
    this->env_.p_mutation = dflt.p_mutation;

  if (constrained.p_cross < 0.0)
    this->env_.p_cross = dflt.p_cross;

  if (!constrained.brood_recombination)
    this->env_.brood_recombination = *dflt.brood_recombination;

  // With a small number of training case:
  // * we need every training case;
  // * DSS speed up isn't so sensible;
  // BUT
  // * DSS can help against overfitting.
  if (constrained.dss == trilean::unknown)
  {
    this->env_.dss = d_size > 400 ? trilean::yes : trilean::no;

    if (this->env_.verbosity >= 2)
      std::cout << k_s_info << " DSS set to " << this->env_.dss << '\n';
  }

  if (!constrained.layers)
  {
    if (d_size > 8)
      this->env_.layers = static_cast<unsigned>(std::log(d_size));
    else
      this->env_.layers = dflt.layers;

    if (this->env_.verbosity >= 2)
      std::cout << k_s_info << " Number of layers set to " << this->env_.layers
                << '\n';
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
    if (d_size > 8)
    {
      this->env_.individuals = 2 *
        static_cast<unsigned>(std::pow((std::log2(d_size)), 3)) /
        this->env_.layers;

      if (this->env_.individuals < 4)
        this->env_.individuals = 4;
    }
    else
      this->env_.individuals = dflt.individuals;

    if (this->env_.verbosity >= 2)
      std::cout << k_s_info << " Population size set to "
                << this->env_.individuals << '\n';
  }

  // Note that this setting, once set, will not be changed.
  if (constrained.validation_percentage > 100 &&
      this->env_.validation_percentage > 100)
  {
    if (d_size)
    {
      if (d_size * dflt.validation_percentage < 10000)
        this->env_.validation_percentage = 0;
      else
        this->env_.validation_percentage = dflt.validation_percentage;
    }
    else
      this->env_.validation_percentage = dflt.validation_percentage;

    if (this->env_.verbosity >= 2)
      std::cout << k_s_info << " Validation percentage set to "
                << this->env_.validation_percentage << "%\n";
  }

  if (!constrained.tournament_size)
    this->env_.tournament_size = dflt.tournament_size;

  if (!constrained.mate_zone)
    this->env_.mate_zone = *dflt.mate_zone;

  if (!constrained.generations)
    this->env_.generations = dflt.generations;

  if (!constrained.g_without_improvement)
    this->env_.g_without_improvement = dflt.g_without_improvement;

  if (constrained.arl == trilean::unknown)
    this->env_.arl = dflt.arl;

  assert(this->env_.debug(true, true));
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
void src_search<T, ES>::dss(unsigned generation) const
{
  if (!this->prob_.data())
    return;

  data &d(*this->prob_.data());

  std::uintmax_t weight_sum(0);
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
  const auto s(static_cast<double>(d.size()));
  const auto ratio(std::min(0.6, 0.2 + 100.0 / (s + 100.0)));
  assert(0.2 <= ratio && ratio <= 0.6);
  const auto target_size(s * ratio);
  assert(0.0 <= target_size && target_size <= s);

  data::iterator base(d.begin());
  unsigned count(0);
  for (auto i(d.begin()); i != d.end(); ++i)
  {
    const auto p1(static_cast<double>(weight(*i)) * target_size /
                  static_cast<double>(weight_sum));
    const auto prob(std::min(p1, 1.0));

    if (random::boolean(prob))
    {
      std::iter_swap(base, i);
      ++base;
      ++count;
    }
  }

  d.slice(std::max(count, 10u));
  this->active_eva_->clear(evaluator<T>::all);

  // Selected training examples have their difficulties and ages reset.
  for (auto &i : d)
  {
    i.difficulty = 0;
    i.age        = 1;
  }
}

///
/// \param[in] n number of runs.
/// \return a summary of the search.
///
template<class T, template<class> class ES>
summary<T> src_search<T, ES>::run_nvi(unsigned n)
{
  assert(this->prob_.data());
  auto &data(*this->prob_.data());

  summary<T> overall_summary;
  distribution<fitness_t> fd;

  unsigned best_run(0);
  std::vector<unsigned> good_runs;

  tune_parameters_nvi();

  // For `std::placeholders` and `std::bind` see:
  // <http://en.cppreference.com/w/cpp/utility/functional/placeholders>
  std::function<void (unsigned)> shake;
  if (this->env_.dss == trilean::yes)
    shake = std::bind(&src_search::dss, this, std::placeholders::_1);

  const auto stop(std::bind(&src_search::stop_condition, this,
                            std::placeholders::_1));

  const bool validation(0 < this->env_.validation_percentage &&
                        this->env_.validation_percentage < 100);

  if (validation)
    data.partition(this->env_.validation_percentage);

  auto &eval(*this->active_eva_);  // Just a shorthand

  for (unsigned r(0); r < n; ++r)
  {
    evolution<T, ES> evo(this->env_, eval, stop, shake);
    summary<T> run_summary(evo.run(r));

    // Depending on `validation`, the metrics stored in run_summary.best.score
    // can refer to the training set or to the validation set (anyway they
    // regards the current run).
    if (validation)
    {
      const data::dataset_t backup(data.dataset());

      data.dataset(data::validation);
      eval.clear(run_summary.best.solution);

      run_summary.best.score.fitness = this->fitness(run_summary.best.solution);
      calculate_metrics(run_summary.best.solution, &run_summary.best.score);

      data.dataset(backup);
      eval.clear(run_summary.best.solution);
    }
    else  // not using a validation set
    {
      // If shake is true, the values calculated during the evolution
      // refer to a subset of the available training set. Since we need an
      // overall fitness for comparison, a new calculation has to be
      // performed.
      if (shake)
      {
        data.dataset(data::training);
        data.slice(false);
        eval.clear(run_summary.best.solution);

        run_summary.best.score.fitness =
          this->fitness(run_summary.best.solution);
      }

      calculate_metrics(run_summary.best.solution, &run_summary.best.score);
    }

    print_resume(validation, run_summary.best.score);

    if (r == 0 ||
        run_summary.best.score.fitness > overall_summary.best.score.fitness)
    {
      overall_summary.best = run_summary.best;
      best_run = r;
    }

    // We use accuracy or fitness (or both) to identify successful runs.
    const bool solution_found(
      dominating(run_summary.best.score.fitness, this->env_.threshold.fitness) &&
      run_summary.best.score.accuracy >= this->env_.threshold.accuracy);

    if (solution_found)
    {
      overall_summary.last_imp += run_summary.last_imp;

      good_runs.push_back(r);
    }

    if (isfinite(run_summary.best.score.fitness))
      fd.add(run_summary.best.score.fitness);

    overall_summary.elapsed += run_summary.elapsed;

    if (this->env_.arl == trilean::yes)
    {
      this->prob_.env.sset->reset_adf_weights();
      arl(run_summary.best.solution);
    }

    assert(good_runs.empty() ||
           std::find(good_runs.begin(), good_runs.end(), best_run) !=
           good_runs.end());
    log(overall_summary, fd, good_runs, best_run, n);
  }

  return overall_summary;
}

///
/// \param[in] validation is it a validation or training resume?
/// \param[in] fit fitness reached in the current run.
/// \param[in] m metrics relative to the current run.
///
template<class T, template<class> class ES>
void src_search<T, ES>::print_resume(
  bool validation, const typename summary<T>::measurements &m) const
{
  if (this->env_.verbosity >= 2)
  {
    const std::string ds(validation ? " Validation" : " Training");

    std::cout << k_s_info << ds << " fitness: " << m.fitness << '\n';
    if (0 <= m.accuracy && m.accuracy <= 1.0)
      std::cout << k_s_info << ds << " accuracy: " << 100.0 * m.accuracy
                << '%';

    std::cout << "\n\n";
  }
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
void src_search<T, ES>::log(const summary<T> &run_sum,
                            const distribution<fitness_t> &fd,
                            const std::vector<unsigned> &good_runs,
                            unsigned best_run, unsigned runs)
{
  // Summary logging.
  if (this->env_.stat.summary)
  {
    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best.solution.list(best_list);
    run_sum.best.solution.tree(best_tree);
    run_sum.best.solution.graphviz(best_graph);

    const std::string path("vita.");
    const std::string summary(path + "summary.");

    const auto solutions(static_cast<unsigned>(good_runs.size()));

    boost::property_tree::ptree pt;
    pt.put(summary + "success_rate", runs ?
           static_cast<double>(solutions) / static_cast<double>(runs) : 0);
    pt.put(summary + "elapsed_time", run_sum.elapsed);
    pt.put(summary + "mean_fitness", fd.mean());
    pt.put(summary + "standard_deviation", fd.standard_deviation());

    pt.put(summary + "best.fitness", run_sum.best.score.fitness);
    pt.put(summary + "best.accuracy", run_sum.best.score.accuracy);
    pt.put(summary + "best.run", best_run);
    pt.put(summary + "best.solution.tree", best_tree.str());
    pt.put(summary + "best.solution.list", best_list.str());
    pt.put(summary + "best.solution.graph", best_graph.str());

    for (const auto &p : good_runs)
      pt.add(summary + "solutions.runs.run", p);
    pt.put(summary + "solutions.found", solutions);
    pt.put(summary + "solutions.avg_depth",
           solutions ? run_sum.last_imp / solutions : 0);

    pt.put(summary + "other.evaluator", this->active_eva_->info());

    const std::string f_sum(this->env_.stat.dir + "/" +
                            this->env_.stat.sum_name);

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

  // Test set results logging.
  vita::data &data(*this->prob_.data());
  if (data.size(data::test))
  {
    const data::dataset_t backup(data.dataset());
    data.dataset(data::test);

    const auto lambda(this->lambdify(run_sum.best.solution));

    std::ofstream tf(this->env_.stat.dir + "/" + this->env_.stat.tst_name);
    for (const auto &example : data)
      tf << lambda->name((*lambda)(example)) << '\n';

    data.dataset(backup);
  }
}

///
/// \param[in] id numerical id of the evaluator to be activated.
/// \param[in] msg input parameters for the evaluator constructor.
/// \return `true` if the active evaluator has been changed.
///
/// \note
/// If the evaluator `id` is not compatible with the problem type the
/// function return `false` and the active evaluator stays the same.
///
template<class T, template<class> class ES>
bool src_search<T, ES>::set_evaluator(evaluator_id id, const std::string &msg)
{
  auto &data(*this->prob_.data());

  if (data.classes() > 1)
  {
    switch (id)
    {
    case evaluator_id::bin:
      search<T, ES>::set_evaluator(
	    vita::make_unique<binary_evaluator<T>>(data));
      return true;

    case evaluator_id::dyn_slot:
      {
        auto x_slot(static_cast<unsigned>(msg.empty() ? 10ul
                                                      : std::stoul(msg)));
        search<T, ES>::set_evaluator(
          vita::make_unique<dyn_slot_evaluator<T>>(data, x_slot));
      }
      return true;

    case evaluator_id::gaussian:
      search<T, ES>::set_evaluator(
	    vita::make_unique<gaussian_evaluator<T>>(data));
      return true;

    default:
      return false;
    }
  }
  else // Symbolic regression
  {
    switch (id)
    {
    case evaluator_id::count:
      search<T, ES>::set_evaluator(
	    vita::make_unique<count_evaluator<T>>(data));
      return true;

    case evaluator_id::mae:
      search<T, ES>::set_evaluator(
	    vita::make_unique<mae_evaluator<T>>(data));
      return true;

    case evaluator_id::rmae:
      search<T, ES>::set_evaluator(
	    vita::make_unique<rmae_evaluator<T>>(data));
      return true;

    case evaluator_id::mse:
      search<T, ES>::set_evaluator(
	    vita::make_unique<mse_evaluator<T>>(data));
      return true;

    default:
      return false;
    }
  }
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the object passes the internal consistency check.
///
template<class T, template<class> class ES>
bool src_search<T, ES>::debug_nvi(bool verbose) const
{
  if (p_symre == evaluator_id::undefined)
  {
    if (verbose)
      std::cerr << "Undefined ID for preferred sym.reg. evaluator.\n";
    return false;
  }

  if (p_class == evaluator_id::undefined)
  {
    if (verbose)
      std::cerr << "Undefined ID for preferred classification evaluator.\n";
    return false;
  }

  return true;
}

#endif  // Include guard
