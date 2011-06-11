/**
 *
 *  \file search.cc
 *
 *  \author Manlio Morini
 *  \date 2011/05/29
 *
 *  This file is part of VITA
 *
 */

#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "search.h"
#include "adf.h"
#include "evolution.h"

namespace vita
{

  ///
  /// \param[in] prob a \c problem used for search initialization.
  ///
  search::search(problem &prob) : _prob(prob)
  {
    assert(prob.check());

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
  void
  search::arl(const individual &base, evolution &evo)
  {
    const unsigned arl_args(0);

    const fitness_t base_fit(evo.fitness(base));
    if (!is_bad(base_fit))
    {
      const std::string f_adf(_prob.env.stat_dir + "/" +
                              environment::arl_filename);
      std::ofstream adf_l(f_adf.c_str(),std::ios_base::app);

      if (_prob.env.stat_arl && adf_l.good())
      {
        unsigned i(0);
        for (const adf_0 *f = _prob.env.sset.get_adf0(i);
             f;
             f = _prob.env.sset.get_adf0(++i))
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
          const double d_f( base_fit - evo.fitness(base.destroy_block(*i)) );

          // Semantic introns cannot be building blocks.
          if (!is_bad(d_f) && std::fabs(base_fit/10.0) < d_f)
          {
            vita::symbol *p;
            if (arl_args)
            {
              std::vector<symbol_t> types;
              candidate_block.generalize(arl_args,0,&types);
              p = new vita::adf_n(candidate_block,types,10);
            }
            else
              p = new vita::adf_0(candidate_block,100);
            _prob.env.insert(p);

            if (_prob.env.stat_arl && adf_l.good())
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
  individual
  search::run(bool verbose, unsigned n, fitness_t success_f)
  {
    summary overall_run_sum;
    distribution<fitness_t> fd;
    unsigned best_run(0);

    unsigned solutions(0);

    summary previous;
    for (unsigned i(0); i < n; ++i)
    {
      evolution evo(&_prob.env,_prob.get_evaluator());
      const summary s(evo(verbose,i));

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

      if (_prob.env.arl && best_run == i)
      {
        _prob.env.sset.reset_adf_weights();
        arl(s.best,evo);
      }

      if (_prob.env.stat_summary)
        log(overall_run_sum,fd,solutions,best_run,n);

      previous = s;
    }

    return overall_run_sum.best;
  }

  ///
  /// \param[in] run_sum summary information regarding the search.
  /// \param[in] fd statistics about population fitness.
  /// \param[in] solutions number of solutions found.
  /// \param[in] best_run best run of the search.
  /// \param[in] runs number of runs performed.
  /// \return true if the write operation succeed.
  ///
  void
  search::log(const summary &run_sum, const distribution<fitness_t> &fd,
              unsigned solutions, unsigned best_run, unsigned runs) const
  {
    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best.list(best_list);
    run_sum.best.tree(best_tree);
    run_sum.best.graphviz(best_graph);

    const std::string path("vita.");
    const std::string summary(path+"summary.");

    boost::property_tree::ptree pt;
    pt.put(summary+"success_rate",runs ? double(solutions)/double(runs) : 0);
    pt.put(summary+"best.fitness",run_sum.f_best);
    pt.put(summary+"best.times_reached",solutions);
    pt.put(summary+"best.run",best_run);
    pt.put(summary+"best.avg_depth_found",solutions
                        ? unsigned(double(run_sum.last_imp)/double(solutions))
                        : 0);
    pt.put(summary+"best.mean_fitness",fd.mean);
    pt.put(summary+"best.standard_deviation",fd.standard_deviation());
    pt.put(summary+"best.individual.tree",best_tree.str());
    pt.put(summary+"best.individual.list",best_list.str());
    pt.put(summary+"best.individual.graph",best_graph.str());
    pt.put(summary+"ttable.hits",run_sum.ttable_hits);
    pt.put(summary+"ttable.probes",run_sum.ttable_probes);

    const std::string f_sum(_prob.env.stat_dir + "/" +
                            environment::sum_filename);

    _prob.env.log(&pt,path);

    using namespace boost::property_tree::xml_parser;
    write_xml(f_sum,pt,std::locale(),xml_writer_make_settings(' ',2));
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool
  search::check() const
  {
    return true;
  }

}  // namespace vita
