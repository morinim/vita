/**
 *
 *  \file search.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <fstream>
#include <list>
#include <string>
#include <vector>

#include <boost/property_tree/xml_parser.hpp>

#include "kernel/search.h"
#include "kernel/adf.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

namespace vita
{
  ///
  /// \param[in] prob a \c problem used for search initialization.
  ///
  search::search(problem *const prob) : env_(prob->env), prob_(prob)
  {
    assert(prob->check(true));

    assert(check(true));
  }

  ///
  /// \param[in] base \a individual in which we are looking for building blocks.
  /// \param[in] evo evolution up to now.
  ///
  /// Adaptive Representation through Learning (ARL). The algorithm extract
  /// common knowledge (building blocks) emerging during the evolutionary
  /// process and acquires the necessary structure for solving the problem
  /// (see ARL - Justinian P. Rosca and Dana H. Ballard).
  ///
  void search::arl(const individual &base, evolution &evo)
  {
    const fitness_t base_fit(evo.fitness(base));
    if (std::isfinite(base_fit))
    {
      const std::string filename(env_.stat_dir + "/" +
                                 environment::arl_filename);
      std::ofstream log(filename.c_str(), std::ios_base::app);
      if (env_.stat_arl && log.good())
      {
        for (unsigned i(0); i < env_.sset.adts(); ++i)
        {
          const symbol *f(env_.sset.get_adt(i).get());
          log << f->display() << ' ' << f->weight << std::endl;
        }
        log << std::endl;
      }

      const unsigned adf_args(0);
      std::list<locus> block_locus(base.blocks());
      for (auto i(block_locus.begin()); i != block_locus.end(); ++i)
      {
        individual candidate_block(base.get_block(*i));

        // Building blocks should be simple.
        if (candidate_block.eff_size() <= 5 + adf_args)
        {
          const double d_f(base_fit -
                           evo.fitness(base.destroy_block((*i)[0])));

          // Semantic introns cannot be building blocks.
          if (std::isfinite(d_f) && std::fabs(base_fit/10.0) < d_f)
          {
            symbol_ptr p;
            if (adf_args)
            {
              std::vector<locus> loci;
              individual generalized(candidate_block.generalize(adf_args,
                                                                &loci));
              std::vector<category_t> categories(loci.size());
              for (unsigned j(0); j < loci.size(); ++j)
                categories[j] = loci[j][locus_category];

              p = std::make_shared<vita::adf>(generalized, categories, 10);
            }
            else  // !adf_args
              p = std::make_shared<vita::adt>(candidate_block, 100);
            env_.insert(p);

            if (env_.stat_arl && log.good())
            {
              log << p->display() << " (Base: " << base_fit
                  << "  DF: " << d_f
                  << "  Weight: " << std::fabs(d_f / base_fit) * 100.0 << "%)"
                  << std::endl;
              candidate_block.list(log);
              log << std::endl;
            }
          }
        }
      }
    }
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
  /// \li firstly 'difficult' cases;
  /// \li secondly cases which have not been looked at for several generations.
  ///
  void search::dss(unsigned generation) const
  {
    data *const d(prob_->data());

    if (d)
    {
      std::function<boost::uint64_t (const data::example &)>
        weight([](const data::example &v) -> boost::uint64_t
               {
                 return v.difficulty + v.age * v.age * v.age;
               });

      boost::uint64_t weight_sum(0);
      d->dataset(data::training);
      for (data::iterator i(d->begin()); i != d->end(); ++i)
      {
        if (generation == 0)  // preliminary setup for generation 0
        {
          i->difficulty = 0;
          i->age        = 1;
        }
        else
          ++i->age;

        weight_sum += weight(*i);
      }

      // Select a subset of the training examples.
      // Training examples, contained in d, are partitioned into two subsets
      // by multiple swaps (first subset: [0, count[,  second subset:
      // [count, d.size()[).
      // Note that the actual size of the selected subset (count) is not fixed
      // and, in fact, it averages slightly above target_size (Gathercole and
      // Ross felt that this might improve performance).
      const boost::uint64_t target_size(d->size() * 20 / 100);
      data::iterator base(d->begin());
      unsigned count(0);
      for (data::iterator i(d->begin()); i != d->end(); ++i)
      {
        const double prob(
          std::min(static_cast<double>(weight(*i)) * target_size / weight_sum,
                   1.0));

        if (random::boolean(prob))
        {
          std::iter_swap(base, i);
          ++base;
          ++count;
        }
      }

      //d->sort(
      //  [](const data::example &v1, const data::example &v2) -> bool
      //  {
      //    double w1(v1.difficulty + v1.age * v1.age * v1.age);
      //    double w2(v2.difficulty + v2.age * v2.age * v2.age);
      //    w1 *= random::between<double>(0.9, 1.1);
      //    w2 *= random::between<double>(0.9, 1.1);
      //    return w1 > w2;
      //  });

      d->dataset(data::training, std::max(10u, count));
      prob_->get_evaluator()->clear();

      // Selected training examples have their difficulties and ages reset.
      for (data::iterator i(d->begin()); i != d->end(); ++i)
      {
        i->difficulty = 0;
        i->age        = 1;
      }
    }
  }

  ///
  /// Parameter tuning is a typical approach to algorithm design. Such tuning
  /// is done by experimenting with different values and selecting the ones
  /// that give the best results on the test problems at hand. However, the
  /// number of possible parameters and their different values means that this
  /// is a very complex and time-consuming task; it is something we do
  /// not/ want users to worry about (power users can force many parameters, but
  /// our idea is "simple by default").
  /// It has been formally proven, in the No-Free-Lunch theorem, that it is
  /// impossible to tune a search algorithm such that it will have optimal
  /// settings for all possible problems, but parameters can be properly
  /// set for a given problem.
  /// This function tries to guess a good starting point and changes its hint
  /// after every run. The code is a mix of black magic, experience, common
  /// logic and randomness but it seems reasonable.
  /// \see "Parameter Setting in Evolutionary Algorithms" (F.G. Lobo, C.F. Lima,
  ///      Z. Michalewicz) - Springer
  ///
  void search::tune_parameters()
  {
    const environment dflt(true);

    if (!prob_->env.code_length)
      env_.code_length = *dflt.code_length;

    if (boost::indeterminate(prob_->env.elitism))
      env_.elitism = dflt.elitism;

    if (!prob_->env.p_mutation)
      env_.p_mutation = *dflt.p_mutation;

    if (!prob_->env.p_cross)
      env_.p_cross = *dflt.p_cross;

    if (!prob_->env.brood_recombination)
      env_.brood_recombination = *dflt.brood_recombination;

    if (boost::indeterminate(prob_->env.dss))
    {
      data *const d(prob_->data());

      env_.dss = d && d->size() > 200;
    }

    if (!prob_->env.individuals)
      env_.individuals = *dflt.individuals;

    if (!prob_->env.tournament_size)
      env_.tournament_size = *dflt.tournament_size;

    if (!prob_->env.mate_zone)
      env_.mate_zone = *dflt.mate_zone;

    if (!prob_->env.g_since_start)
      env_.g_since_start = *dflt.g_since_start;

    if (!prob_->env.g_without_improvement)
      env_.g_without_improvement = *dflt.g_without_improvement;

    if (boost::indeterminate(prob_->env.arl))
      env_.arl = dflt.arl;

    assert(env_.check(true, true));
  }

  ///
  /// \param[in] verbose prints verbose informations while running.
  /// \param[in] n number of runs.
  /// \return best individual found.
  ///
  const individual &search::run(bool verbose, unsigned n)
  {
    assert(prob_->threashold.fitness || prob_->threashold.accuracy);

    // This is used in comparisons between fitnesses: we considered values
    // distinct only when their distance is greater than tolerance.
    const fitness_t tolerance(0.0001);

    summary overall_summary;
    distribution<fitness_t> fd;

    unsigned solutions(0);

    std::list<unsigned> good_runs;
    score_t run_best_score;

    // For std::placeholders and std::bind see:
    //  http://en.cppreference.com/w/cpp/utility/functional/placeholders
    std::function<void (unsigned)> shake_data;
    if (env_.dss)
      shake_data = std::bind(&search::dss, this, std::placeholders::_1);

    tune_parameters();

    for (unsigned run(0); run < n; ++run)
    {
      evolution evo(env_, prob_->get_evaluator(), shake_data);
      summary s(evo(verbose, run,
                    selection_factory::k_tournament,
                    operation_factory::k_crossover_mutation,
                    replacement_factory::k_tournament));

      // If \c shake_data == \c true, the values returned by the evolution
      // object refer to a subset of the available dataset. Since we need an
      // overall score, a new calculation have to be performed.
      if (shake_data)
      {
        prob_->data()->dataset(data::training);
        prob_->get_evaluator()->clear();
        run_best_score = (*prob_->get_evaluator())(s.best->ind);

        if (verbose)
        {
          std::cout << run_best_score.fitness;

          if (run_best_score.accuracy >= 0.0)
            std::cout << " (" << 100.0 * run_best_score.accuracy << "%)"
                      << std::endl;
        }
      }
      else
        run_best_score = s.best->score;

      if (run == 0)
        overall_summary.best = {s.best->ind, run_best_score};

      // We can use accuracy or fitness to identify successfully runs (it
      // depends on prob_->threashold).
      const bool solution_found(
        prob_->threashold.fitness ?
        run_best_score.fitness >= *prob_->threashold.fitness :
        run_best_score.accuracy >= *prob_->threashold.accuracy);

      if (solution_found)
      {
        ++solutions;
        overall_summary.last_imp += s.last_imp;
      }

      // Good is true when the present individual/solution satisfies (is
      // greater than OR equal to) the current threashold criterion (accuracy
      // OR fitness).
      const bool good(
        prob_->threashold.fitness ?

        run_best_score.fitness + tolerance >=
        overall_summary.best->score.fitness :

        run_best_score.accuracy >= overall_summary.best->score.accuracy);

      if (good)
      {
        // Well, we have found a good individual, is it a new best?

        if (run_best_score.fitness > overall_summary.best->score.fitness +
            tolerance)
        {
          overall_summary.best->score.fitness = run_best_score.fitness;

          if (prob_->threashold.fitness)
          {
            overall_summary.best->ind = s.best->ind;
            good_runs.clear();
          }
        }

        if (run_best_score.accuracy > overall_summary.best->score.accuracy)
        {
          overall_summary.best->score.accuracy = run_best_score.accuracy;

          if (prob_->threashold.accuracy)
          {
            overall_summary.best->ind = s.best->ind;
            good_runs.clear();
          }
        }

        good_runs.push_back(run);
      }

      if (std::isfinite(run_best_score.fitness))
        fd.add(run_best_score.fitness);

      overall_summary.ttable_hits += s.ttable_hits;
      overall_summary.ttable_probes += s.ttable_probes;
      overall_summary.speed = overall_summary.speed +
        (s.speed - overall_summary.speed) / (run + 1);

      if (env_.arl && good_runs.front() == run)
      {
        env_.sset.reset_adf_weights();
        arl(s.best->ind, evo);
      }

      if (env_.stat_summary)
        log(overall_summary, fd, good_runs, solutions, n);
    }

    return overall_summary.best->ind;
  }

  ///
  /// \param[in] run_sum summary information regarding the search.
  /// \param[in] fd statistics about population fitness.
  /// \param[in] solutions number of solutions found.
  /// \param[in] best_runs list of the best runs of the search.
  /// \param[in] runs number of runs performed.
  /// \return \c true if the write operation succeed.
  ///
  void search::log(const summary &run_sum, const distribution<fitness_t> &fd,
                   const std::list<unsigned> &best_runs, unsigned solutions,
                   unsigned runs) const
  {
    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best->ind.list(best_list);
    run_sum.best->ind.tree(best_tree);
    run_sum.best->ind.graphviz(best_graph);

    const std::string path("vita.");
    const std::string summary(path + "summary.");

    boost::property_tree::ptree pt;
    pt.put(summary+"success_rate", runs ?
           static_cast<double>(solutions) / static_cast<double>(runs) : 0);
    pt.put(summary+"speed", run_sum.speed);
    pt.put(summary+"mean_fitness", fd.mean);
    pt.put(summary+"standard_deviation", fd.standard_deviation());
    pt.put(summary+"best.fitness", run_sum.best->score.fitness);
    pt.put(summary+"best.accuracy", run_sum.best->score.accuracy);
    pt.put(summary+"best.times_reached", best_runs.size());
    pt.put(summary+"best.avg_depth_found", solutions
           ? static_cast<unsigned>(static_cast<double>(run_sum.last_imp) /
                                   static_cast<double>(solutions))
           : 0);
    for (auto p(best_runs.cbegin()); p != best_runs.cend(); ++p)
      pt.add(summary+"best.runs.run", *p);
    pt.put(summary+"best.individual.tree", best_tree.str());
    pt.put(summary+"best.individual.list", best_list.str());
    pt.put(summary+"best.individual.graph", best_graph.str());
    pt.put(summary+"ttable.hits", run_sum.ttable_hits);
    pt.put(summary+"ttable.probes", run_sum.ttable_probes);

    const std::string f_sum(env_.stat_dir + "/" + environment::sum_filename);

    env_.log(&pt, path);

    using namespace boost::property_tree;
    write_xml(f_sum, pt, std::locale(), xml_writer_make_settings(' ', 2));
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool search::check(bool verbose) const
  {
    return prob_->check(verbose);
  }
}  // namespace vita
