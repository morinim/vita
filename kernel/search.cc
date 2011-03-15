/**
 *
 *  \file search.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
 *
 */
  
#include <fstream>

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
            const std::string f_adf(_prob.env.stat_dir + "/adf");
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
  /// \return best individual found.
  ///
  individual
  search::run(bool verbose, unsigned n)
  {   
    if (_prob.env.stat_env)
    {
      const std::string filename(_prob.env.stat_dir + "/environment");
      std::ofstream logs(filename.c_str());

      if (logs.good())
	_prob.env.log(logs);
    }

    summary run_sum;
    distribution<fitness_t> fd;

    // Consecutive runs reporting the same best individuals.
    unsigned best_counter(0);

    population p(_prob.env);
    evolution evo(_prob.env,p,*_prob.get_evaluator());
    for (unsigned i(0); i < n; ++i)
    {
      if (i)
	p = population(_prob.env);

      const summary s(evo.run(verbose));

      if (i == 0)
      {
	best_counter   =        1;
	run_sum.best   =   s.best;
	run_sum.f_best = s.f_best;
      }
      
      const bool almost_equal( distance(s.f_best,run_sum.f_best) <= 
			       float_epsilon );

      if (almost_equal && i>0)
      {
	++best_counter;
	run_sum.last_imp += s.last_imp;
      }

      if (run_sum.f_best < s.f_best)
      {
	if (!almost_equal)
	{
	  best_counter = 1;
	  run_sum.last_imp = s.last_imp;
	}

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
    {
      const std::string f_sum(_prob.env.stat_dir + "/summary");
      std::ofstream sum(f_sum.c_str());
      if (sum.good())
	sum << run_sum.f_best 
	    << ' ' << fd.mean
	    << ' ' << fd.standard_deviation()
	    << ' ' << best_counter
	    << ' ' << run_sum.last_imp / best_counter
	    << ' ' << run_sum.ttable_hits*100 / run_sum.ttable_probes
	    << std::endl;

      sum << std::endl << "{Best individual (list)}" << std::endl;
      run_sum.best.list(sum);
      sum << std::endl <<"{Best individual (tree)}" << std::endl;
      run_sum.best.tree(sum);
      sum << std::endl <<"{Best individual (graphviz)}" << std::endl;
      run_sum.best.graphviz(sum);
    }

    return run_sum.best;
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
