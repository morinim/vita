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

    const fitness_t base_fit(evo.fitness(base));
    if (!is_bad(base_fit))
    {
      const std::string f_adf(prob_->env.stat_dir + "/" +
                              environment::arl_filename);
      std::ofstream adf_l(f_adf.c_str(), std::ios_base::app);

      if (prob_->env.stat_arl && adf_l.good())
      {
        unsigned i(0);
        for (const symbol *f(prob_->env.sset.get_adt(i).get());
             f;
             f = prob_->env.sset.get_adt(++i).get())
          adf_l << f->display() << ' ' << f->weight << std::endl;
        adf_l << std::endl;
      }

      std::list<unsigned> block_index(base.blocks());
      for (std::list<unsigned>::const_iterator i(block_index.begin());
           i != block_index.end();
           ++i)
      {
        individual candidate_block(base.get_block(*i));

        // Building blocks should be simple.
        if (candidate_block.eff_size() <= 5+arl_args)
        {
          const double d_f(base_fit - evo.fitness(base.destroy_block(*i)));

          // Semantic introns cannot be building blocks.
          if (!is_bad(d_f) && std::fabs(base_fit/10.0) < d_f)
          {
            symbol_ptr p;
            if (arl_args)
            {
              std::vector<symbol_t> types;
              individual generalized(candidate_block.generalize(arl_args,
                                                                0, &types));
              p.reset(new vita::adf(generalized, types, 10));
            }
            else
              p.reset(new vita::adt(candidate_block, 100));
            prob_->env.insert(p);

            if (prob_->env.stat_arl && adf_l.good())
            {
              adf_l << p->display() << " (Base: " << base_fit
                    << "  DF: " << d_f
                    << "  Weight: " << std::fabs(d_f/base_fit)*100.0 << "%)"
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
  /// \param[in] verbose prints verbose informations while running.
  /// \param[in] n number of runs.
  /// \param[in] success_f when an individual reaches this fitness it is
  ///                      considered a solution.
  /// \return best individual found.
  ///
  const individual &search::run(bool verbose, unsigned n, fitness_t success_f)
  {
    summary overall_run_sum;
    distribution<fitness_t> fd;
    unsigned best_run(0);

    unsigned solutions(0);

    summary previous;
    for (unsigned i(0); i < n; ++i)
    {
      evolution evo(&prob_->env, prob_->get_evaluator());
      const summary s(evo(verbose, i));

      if (i == 0)
      {
        overall_run_sum.best   =   s.best;
        overall_run_sum.f_best = s.f_best;
      }

      const bool found(s.f_best >= success_f);
      if (found)
      {
        ++solutions;
        overall_run_sum.last_imp += s.last_imp;
      }

      if (overall_run_sum.f_best < s.f_best)
      {
        overall_run_sum.best   =   s.best;
        overall_run_sum.f_best = s.f_best;
        best_run               =        i;
      }

      fd.add(s.f_best);
      overall_run_sum.ttable_hits += s.ttable_hits;
      overall_run_sum.ttable_probes += s.ttable_probes;

      if (prob_->env.arl && best_run == i)
      {
        prob_->env.sset.reset_adf_weights();
        arl(*s.best, evo);
      }

      if (prob_->env.stat_summary)
        log(overall_run_sum, fd, solutions, best_run, n);

      previous = s;
    }

    return *overall_run_sum.best;
  }

  ///
  /// \param[in] run_sum summary information regarding the search.
  /// \param[in] fd statistics about population fitness.
  /// \param[in] solutions number of solutions found.
  /// \param[in] best_run best run of the search.
  /// \param[in] runs number of runs performed.
  /// \return true if the write operation succeed.
  ///
  void search::log(const summary &run_sum, const distribution<fitness_t> &fd,
                   unsigned solutions, unsigned best_run, unsigned runs) const
  {
    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best->list(best_list);
    run_sum.best->tree(best_tree);
    run_sum.best->graphviz(best_graph);

    const std::string path("vita.");
    const std::string summary(path+"summary.");

    boost::property_tree::ptree pt;
    pt.put(summary+"success_rate", runs ?
           static_cast<double>(solutions) / static_cast<double>(runs) : 0);
    pt.put(summary+"best.fitness", run_sum.f_best);
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

    using namespace boost::property_tree::xml_parser;
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
