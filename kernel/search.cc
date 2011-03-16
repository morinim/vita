/**
 *
 *  \file search.cc
 *
 *  \author Manlio Morini
 *  \date 2011/03/14
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
  /// \param[in] prob
  ///
  search::search(problem &prob) : _prob(prob)
  {
    assert(prob.check());

    assert(check());
  }

  ///
  /// \param[in] candidate individual in which we are looking for building
  ///                      blocks.
  /// \param[in] evo evolution up to now.
  ///
  /// Adaptive Representation through Learning (ARL). The algorithm extract
  /// common knowledge (building blocks) emerging during the evolutionary 
  /// process and acquires the necessary structure for solving the problem 
  /// (see ARL - Justinian P. Rosca and Dana H. Ballard).
  ///
  void
  search::arl(const individual &candidate, evolution &evo)
  {
    const unsigned arl_args(2);

    const fitness_t base_f(evo.fitness(candidate));
    std::list<unsigned> bl(candidate.blocks());

    _prob.env.sset.reset_adf_weights();

    for (std::list<unsigned>::const_iterator i(bl.begin()); i != bl.end(); ++i)
    {
      individual candidate_block(candidate.get_block(*i));

      // Building blocks should be simple.
      if (candidate_block.eff_size() <= 5+arl_args)
      {
        const double d_f( base_f - evo.fitness(candidate.destroy_block(*i)) );

        // Semantic introns cannot be building blocks.
        if (!is_bad(base_f) && !is_bad(d_f) &&
            std::fabs(base_f/10.0) < d_f)
        {
          std::vector<symbol_t> types;
          candidate_block.generalize(arl_args,0,&types);
          vita::adf *const p = new vita::adf(candidate_block,types,10);
          _prob.env.insert(p);
        
          if (_prob.env.stat_arl)
          {
            const std::string f_adf(_prob.env.stat_dir + "/" + 
                                    environment::arl_filename);
            std::ofstream adf_l(f_adf.c_str(),std::ios_base::app);
            if (adf_l.good())
            {
              adf_l << p->display() << " (DF: " << d_f << ')' << std::endl;
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
    if (_prob.env.stat_env)
      _prob.env.log();

    summary run_sum;
    distribution<fitness_t> fd;

    unsigned solutions(0);

    population p(_prob.env);
    evolution evo(_prob.env,p,_prob.get_evaluator());
    for (unsigned i(0); i < n; ++i)
    {
      if (i)
	p = population(_prob.env);

      const summary s(evo.run(verbose));

      if (i == 0)
      {
	run_sum.best   =   s.best;
	run_sum.f_best = s.f_best;
      }
      
      const bool ok(s.f_best >= success_f);

      if (ok)
      {
	++solutions;
	run_sum.last_imp += s.last_imp;
      }

      if (run_sum.f_best < s.f_best)
      {
	run_sum.best   =   s.best;
	run_sum.f_best = s.f_best;
      }
     
      fd.add(s.f_best);
      run_sum.ttable_hits += s.ttable_hits;
      run_sum.ttable_probes += s.ttable_probes;

      if (_prob.env.arl)
        arl(run_sum.best,evo);
    }

    if (_prob.env.stat_summary)
      log(run_sum,fd,solutions,n);

    return run_sum.best;
  }

  ///
  /// \param[in] run_sum summary information regarding the search.
  /// \param[in] fd statistics about population fitness.
  /// \param[in] solutions number of solutions found.
  /// \param[in] runs number of runs performed.
  /// \return true if the write operation succeed.
  ///
  void
  search::log(const summary &run_sum, const distribution<fitness_t> &fd,
              unsigned solutions, unsigned runs) const
  {
    boost::property_tree::ptree pt;

    std::ostringstream best_list, best_tree, best_graph;
    run_sum.best.list(best_list);
    run_sum.best.tree(best_tree);
    run_sum.best.graphviz(best_graph);    

    pt.put("summary.success_rate",runs ? double(solutions)/double(runs) : 0);
    pt.put("summary.best.fitness",run_sum.f_best);
    pt.put("summary.best.times_reached",solutions);
    pt.put("summary.best.avg_depth_found",solutions 
                        ? unsigned(double(run_sum.last_imp)/double(solutions))
                        : 0);
    pt.put("summary.best.individual.tree",best_tree.str());
    pt.put("summary.best.individual.list",best_list.str());
    pt.put("summary.best.individual.graph",best_graph.str());
    pt.put("summary.population.mean_fitness",fd.mean);
    pt.put("summary.population.standard_deviation",fd.standard_deviation());
    pt.put("summary.ttable.found_perc",run_sum.ttable_probes 
                        ? run_sum.ttable_hits*100 / run_sum.ttable_probes
                        : 0);

    const std::string f_sum(_prob.env.stat_dir + "/" + 
                            environment::sum_filename);

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
