/**
 *
 *  \file search.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <fstream>
#include <list>
#include <string>
#include <vector>

#include "kernel/search.h"
#include "kernel/adf.h"
#include "kernel/evolution.h"
#include "kernel/problem.h"

namespace vita
{
  ///
  /// \param[in] prob a \c problem used for search initialization.
  ///
  search::search(problem *const prob) : prob_(prob)
  {
    assert(prob->check());

    assert(check());
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
    const unsigned arl_args(0);

    const fitness_t base_fit(evo.fitness(base).first);
    if (std::isfinite(base_fit))
    {
      const std::string f_adf(prob_->env.stat_dir + "/" +
                              environment::arl_filename);
      std::ofstream adf_l(f_adf.c_str(), std::ios_base::app);

      if (prob_->env.stat_arl && adf_l.good())
      {
        for (unsigned i(0); i < prob_->env.sset.adts(); ++i)
        {
          const symbol *f(prob_->env.sset.get_adt(i).get());
          adf_l << f->display() << ' ' << f->weight << std::endl;
        }
        adf_l << std::endl;
      }

      std::list<locus> block_locus(base.blocks());
      for (auto i(block_locus.begin()); i != block_locus.end(); ++i)
      {
        individual candidate_block(base.get_block(*i));

        // Building blocks should be simple.
        if (candidate_block.eff_size() <= 5 + arl_args)
        {
          const double d_f(base_fit -
                           evo.fitness(base.destroy_block((*i)[0])).first);

          // Semantic introns cannot be building blocks.
          if (std::isfinite(d_f) && std::fabs(base_fit/10.0) < d_f)
          {
            symbol_ptr p;
            if (arl_args)
            {
              std::vector<locus> loci;
              individual generalized(candidate_block.generalize(arl_args,
                                                                &loci));
              std::vector<category_t> categories(loci.size());
              for (unsigned j(0); j != loci.size(); ++j)
                categories[j] = loci[j][locus_category];

              p = std::make_shared<vita::adf>(generalized, categories, 10);
            }
            else
              p = std::make_shared<vita::adt>(candidate_block, 100);
            prob_->env.insert(p);

            if (prob_->env.stat_arl && adf_l.good())
            {
              adf_l << p->display() << " (Base: " << base_fit
                    << "  DF: " << d_f
                    << "  Weight: " << std::fabs(d_f / base_fit) * 100.0 << "%)"
                    << std::endl;
              candidate_block.list(adf_l);
              adf_l << std::endl;
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
  /// \li firstly 'difficult' cases:
  /// \li secondly cases which have not been looked at for several generations.
  ///
  void search::dss(unsigned generation)
  {
    data *const d(prob_->get_data());

    if (d && d->size() > 200)
    {
      std::function<boost::uint64_t (const data::value_type &)>
        weight([](const data::value_type &v) -> boost::uint64_t
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
      //  [](const data::value_type &v1, const data::value_type &v2) -> bool
      //  {
      //    double w1(v1.difficulty + v1.age * v1.age * v1.age);
      //    double w2(v2.difficulty + v2.age * v2.age * v2.age);
      //    w1 *= random::between<double>(0.9, 1.1);
      //    w2 *= random::between<double>(0.9, 1.1);
      //    return w1 > w2;
      //  });

      d->dataset(data::training, std::max(10u, count));
      prob_->get_evaluator()->clear();

      for (data::iterator i(d->begin()); i != d->end(); ++i)
      {
        i->difficulty = 0;
        i->age        = 1;
      }
    }
  }

  ///
  /// \param[in] verbose prints verbose informations while running.
  /// \param[in] n number of runs.
  /// \return best individual found.
  ///
  const individual &search::run(bool verbose, unsigned n)
  {
    summary overall_run_sum;
    distribution<fitness_t> fd;
    unsigned best_run(0);

    unsigned solutions(0);

    summary previous;
    eva_pair pair_run_best;

    for (unsigned run(0); run < n; ++run)
    {
      std::function<void (unsigned)> shake_data;
      if (prob_->env.dss)
        shake_data = std::bind(&search::dss, this, std::placeholders::_1);

      evolution evo(prob_->env, prob_->get_evaluator(), shake_data);
      summary s(evo(verbose, run));

      // If shake_data == true, the values returned by the evolution refer to a
      // subset of the available dataset. Since we need an overall fitness /
      // accuracy, a new calculation have to be performed.
      if (shake_data)
      {
        prob_->get_data()->dataset(data::training);
        prob_->get_evaluator()->clear();
        pair_run_best = (*prob_->get_evaluator())(*s.best_ind);

        if (verbose)
          std::cout << pair_run_best.first << " ("
                    << 100.0 * pair_run_best.second << "%)" << std::endl;
      }
      else
        pair_run_best = s.best_pair;

      if (run == 0)
      {
        overall_run_sum.best_ind  = s.best_ind;
        overall_run_sum.best_pair = pair_run_best;
      }

      const bool found(pair_run_best.first >= prob_->threashold);
      if (found)
      {
        ++solutions;
        overall_run_sum.last_imp += s.last_imp;
      }

      if (overall_run_sum.best_pair.first < pair_run_best.first)
      {
        overall_run_sum.best_ind  =    s.best_ind;
        overall_run_sum.best_pair = pair_run_best;
        best_run                  =           run;
      }

      if (std::isfinite(pair_run_best.first))
        fd.add(pair_run_best.first);

      overall_run_sum.ttable_hits += s.ttable_hits;
      overall_run_sum.ttable_probes += s.ttable_probes;
      overall_run_sum.speed = ((overall_run_sum.speed * run) + s.speed) /
                              (run + 1);

      if (prob_->env.arl && best_run == run)
      {
        prob_->env.sset.reset_adf_weights();
        arl(*s.best_ind, evo);
      }

      if (prob_->env.stat_summary)
        log(overall_run_sum, fd, solutions, best_run, n);

      previous = s;
    }

    return *overall_run_sum.best_ind;
  }

  ///
  /// \param[in] run_sum summary information regarding the search.
  /// \param[in] fd statistics about population fitness.
  /// \param[in] solutions number of solutions found.
  /// \param[in] best_run best run of the search.
  /// \param[in] runs number of runs performed.
  /// \return \c true if the write operation succeed.
  ///
  void search::log(const summary &run_sum, const distribution<fitness_t> &fd,
                   unsigned solutions, unsigned best_run, unsigned runs) const
  {
    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best_ind->list(best_list);
    run_sum.best_ind->tree(best_tree);
    run_sum.best_ind->graphviz(best_graph);

    const std::string path("vita.");
    const std::string summary(path+"summary.");

    boost::property_tree::ptree pt;
    pt.put(summary+"success_rate", runs ?
           static_cast<double>(solutions) / static_cast<double>(runs) : 0);
    pt.put(summary+"speed", run_sum.speed);
    pt.put(summary+"best.fitness", run_sum.best_pair.first);
    pt.put(summary+"best.accuracy", run_sum.best_pair.second);
    pt.put(summary+"best.times_reached", solutions);
    pt.put(summary+"best.run", best_run);
    pt.put(summary+"best.avg_depth_found", solutions
           ? static_cast<unsigned>(static_cast<double>(run_sum.last_imp) /
                                   static_cast<double>(solutions))
           : 0);
    pt.put(summary+"best.mean_fitness", fd.mean);
    pt.put(summary+"best.standard_deviation", fd.standard_deviation());
    pt.put(summary+"best.individual.tree", best_tree.str());
    pt.put(summary+"best.individual.list", best_list.str());
    pt.put(summary+"best.individual.graph", best_graph.str());
    pt.put(summary+"ttable.hits", run_sum.ttable_hits);
    pt.put(summary+"ttable.probes", run_sum.ttable_probes);

    const std::string f_sum(prob_->env.stat_dir + "/" +
                            environment::sum_filename);

    prob_->env.log(&pt, path);

    using namespace boost::property_tree;
    write_xml(f_sum, pt, std::locale(), xml_writer_make_settings(' ', 2));
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool search::check() const
  {
    return prob_->check();
  }
}  // namespace vita
