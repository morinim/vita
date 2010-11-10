/**
 *
 *  \file search.cc
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 */
  
#include <fstream>

#include "search.h"
#include "adf.h"
#include "environment.h"
#include "population.h"

namespace vita
{

  /**
   * search
   * \param[in] e
   */
  search::search(environment &e) : _env(&e)
  {
    assert(e.check());

    assert(check());
  }

  /**
   * arl
   * \param[in] candidate Individual in which we are looking for.
   * \param[in] pop Population including ind.
   *
   * Adaptive Representation through Learning (ARL). The algorithm extract
   * common knowledge (building blocks) emerging during the evolutionary 
   * process and acquires the necessary structure for solving the problem 
   * (see ARL - Justinian P. Rosca and Dana H. Ballard).
   */
  void
  search::arl(const individual &candidate, population &pop)
  {
    const unsigned arl_args(2);

    const fitness_t base_f(pop.fitness(candidate));
    std::list<unsigned> bl(candidate.blocks());

    _env->sset.reset_adf_weights();

    for (std::list<unsigned>::const_iterator i(bl.begin()); i != bl.end(); ++i)
    {
      individual candidate_block(candidate.get_block(*i));

      // Building blocks should be simple.
      if (candidate_block.eff_size() <= 5+arl_args)
      {
        const double d_f( base_f - pop.fitness(candidate.destroy_block(*i)) );

        // Semantic introns cannot be building blocks.
        if (!is_bad(base_f) && !is_bad(d_f) &&
            std::fabs(base_f/10.0) < d_f)
        {
          std::vector<symbol_t> types;
          candidate_block.generalize(arl_args,0,&types);
          vita::adf *const p = new vita::adf(candidate_block,types,10);
          _env->insert(p);
        
          if (_env->stat_arl)
          {
            const std::string f_adf(_env->stat_dir + "/adf");
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

  /**
   * run
   * \param[in] verbose Prints verbose information while running.
   * \param[in] n Number of runs.
   */
  individual
  search::run(bool verbose, unsigned n)
  {   
    if (_env->stat_env)
    {
      const std::string filename(_env->stat_dir + "/environment");
      std::ofstream logs(filename.c_str());

      if (logs.good())
	_env->log(logs);
    }

    summary run_sum;
    distribution<fitness_t> fd;

    // Consecutive runs reporting the same best individuals.
    unsigned best_counter(0);

    for (unsigned i(0); i < n; ++i)
    {
      population p(*_env);

      const summary s(p.evolution(verbose));

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

      if (_env->arl)
        arl(run_sum.best,p);
    }

    if (_env->stat_summary)
    {
      const std::string f_sum(_env->stat_dir + "/summary");
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

  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  bool
  search::check() const
  {
    return _env != 0;
  }

}  // namespace vita
