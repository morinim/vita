/**
 *
 *  \file population.cc
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 */
  
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "environment.h"
#include "population.h"
#include "random.h"

namespace vita
{

  unsigned population::run(0);

  /**
   * population
   * \param[in] e environment.
   */
  population::population(environment &e) : _env(&e), cache(e.ttable_size)
  {
    assert(e.check());

    build();
  }

  /**
   * build
   *
   * Creates a random population.
   */
  void
  population::build()
  {
    _pop.clear();

    for (unsigned i(0); i < _env->individuals; ++i)
      _pop.push_back(individual(*_env,true));

    assert(check());
  }
 
  /**
   * tournament
   * \param[in] target Index of an individual in the population.
   * \param[in] best Are we looking for the best (or the worst) individual?
   * \return Index of the best (worst) individual found.
   * 
   * Tournament selection works by selecting a number of individuals from the 
   * population at random, a tournament, and then selecting only the best of 
   * those individuals.
   * Recall that better individuals have highter fitnesses.
   */
  unsigned
  population::tournament(unsigned target, bool best) const
  {
    const unsigned n(size());
    const unsigned mate_zone(_env->mate_zone);
    const unsigned rounds(best ? _env->par_tournament : _env->rep_tournament);

    unsigned sel(random::ring(target,mate_zone,n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const unsigned j(random::ring(target,mate_zone,n));

      if (best)
      {
        if (fitness(_pop[j]) > fitness(_pop[sel]))
          sel = j;
      }
      else  // worst.
      {
        if (fitness(_pop[j]) < fitness(_pop[sel]))
	  sel = j;
      }
    }

    return sel;
  }

  /**
   * log
   *
   * Saves working / statistical informations in the log files.
   */
  void
  population::log() const
  {
    if ( (_env->stat_period && stats.gen % _env->stat_period == 0) || 
	 _env->stat_dynamic)
    {
      if (_env->stat_period)
      { 
	std::ostringstream f_logs;
	f_logs << _env->stat_dir << '/' << run << '_' << stats.gen;
	std::ofstream logs(f_logs.str().c_str());

	if (logs.good())
	{
	  logs << "{Best individual (fitness)}" << std::endl
	       << stats.f_best << std::endl;

	  logs << std::endl << "{Best individual (list)}" << std::endl;
	  stats.best.list(logs);
	  logs << std::endl <<"{Best individual (tree)}" << std::endl;
	  stats.best.tree(logs);
	  logs << std::endl <<"{Best individual (graphviz)}" << std::endl;
	  stats.best.graphviz(logs);

	  for (unsigned active(0); active <= 1; ++active)
	  {
	    const unsigned long long nf(stats.az.functions(active));
	    const unsigned long long nt(stats.az.terminals(active));
	    const unsigned long long n(nf+nt);
	    const std::string s(active ? "Active" : "Overall");
	    
	    logs << std::endl << '{' << s << " symbol frequency}" 
		 << std::endl;
	    for (analyzer::const_iterator i(stats.az.begin());
		 i != stats.az.end();
		 ++i)
	      logs << (i->first)->display() << ' ' 
		   << i->second.counter[active] << ' ' 
		   << 100*i->second.counter[active]/n << '%' << std::endl;

	    logs << std::endl << '{' << s << " functions}" << std::endl
		 << nf << " (" << nf*100/n << "%)" << std::endl
	         << std::endl << '{' << s << " terminals}" << std::endl
		 << nt << " (" << nt*100/n << "%)" << std::endl;
	  }

	  logs << std::endl << "{Mutations}" << std::endl
	       << stats.mutations << std::endl
	       << std::endl << "{Crossovers}" << std::endl
	       << stats.crossovers 
	       << std::endl << std::endl 
	       << "{Average fitness}" << std::endl
	       << stats.az.fit_dist().mean << std::endl
	       << std::endl << "{Worst fitness}" << std::endl
	       << stats.az.fit_dist().min << std::endl
	       << std::endl << "{Fitness standard deviation}" << std::endl
	       << stats.az.fit_dist().standard_deviation() 
	       << std::endl << std::endl 
	       << "{Average code length}" << std::endl
	       << unsigned(stats.az.length_dist().mean) << std::endl
	       << std::endl << "{Lenth standard deviation}" << std::endl
	       << std::sqrt(stats.az.length_dist().variance) << std::endl
	       << std::endl << "{Max code length}" << std::endl
	       << unsigned(stats.az.length_dist().max) 
	       << std::endl << std::endl 
	       << "{Hit rate}" << std::endl
	       << (cache.probes() ? cache.hits()*100/cache.probes() : 0)
	       << std::endl;
	}
      }

      if (_env->stat_dynamic)
      {
	const std::string f_dynamic(_env->stat_dir + "/dynamic");
	std::ofstream dynamic(f_dynamic.c_str(),std::ios_base::app);
	if (dynamic.good())
	  dynamic << run << ' ' << stats.gen
		  << ' ' << stats.f_best << ' ' << stats.az.fit_dist().mean 
		  << ' ' << stats.az.fit_dist().min
		  << ' ' << stats.az.fit_dist().standard_deviation()
		  << ' ' << unsigned(stats.az.length_dist().mean) 
		  << ' ' << std::sqrt(stats.az.length_dist().variance)
		  << ' ' << unsigned(stats.az.length_dist().max)
		  << ' ' << stats.mutations << ' ' << stats.crossovers
		  << ' ' << stats.az.functions(0) 
		  << ' ' << stats.az.terminals(0)
		  << ' ' << stats.az.functions(1) 
		  << ' ' << stats.az.terminals(1)
		  << ' ' << (cache.probes() ? cache.hits()*100/cache.probes()
			                    : 0)
		  << std::endl;
      }
    }
  }

  /**
   * stop_condition
   * \return true if evolution should be interrupted.
   */
  bool
  population::stop_condition() const
  {
    return 
      (_env->g_since_start && stats.gen > _env->g_since_start) ||

      // We use an accelerated stop condition when all the individuals have
      // the same fitness and after gwi/2 generations the situation isn't
      // changed. 
      ( _env->g_without_improvement && 
        (stats.gen-stats.last_imp > _env->g_without_improvement ||
	(stats.gen-stats.last_imp > _env->g_without_improvement/2 &&
         stats.az.fit_dist().variance <= float_epsilon)) );
  }

  /**
   * pick_stats
   * \param[out] az Repository for the statistical informations.
   *
   * Gathers statistical informations about the elements of the population.
   */
  void
  population::pick_stats(analyzer *const az)
  {
    az->clear();

    for (const_iterator i(begin()); i != end(); ++i)    
    {
      fitness_t f;
      cache.find(*i,&f);

      az->add(*i,f);
    }
  }

  /**
   * evolution
   * \param[in] verbose Prints verbose informations.
   *
   * We enter the genetic programming loop. We begin the loop by choosing a
   * genetic operation: reproduction, mutation or crossover. We then select 
   * the individual(s) to participate in the genetic operation using either 
   * tournament selection. If we are doing reproduction or mutation, we only 
   * select one individual. For crossover, two individuals need to be selected.
   * The genetic operation is then performed and a new offspring individual is
   * created.
   * The offspring is then placed into the original population (steady state)
   * replacing a bad individual. 
   * This whole process repeats until the termination criteria is satisfied.
   * With any luck, this process will produce an individual that solves the 
   * problem at hand. 
   */
  const summary &
  population::evolution(bool verbose)
  {
    stats.clear();
    stats.best   = _pop[0];
    stats.f_best = fitness(stats.best);

    cache.clear();

    std::clock_t start_c(clock());
    for (stats.gen = 0; !stop_condition(); ++stats.gen)
    {
      pick_stats(&stats.az);

      log();

      for (unsigned k(0); k < _pop.size(); ++k)
      {
	if ( verbose && k % std::max(_pop.size()/100,size_t(1)) )
	{
	  std::cout << "Run " << run << '.'
		    << stats.gen << " (" << std::setw(3) << 100*k/_pop.size() 
		    << "%)\r" << std::flush;
	}

	// --------- SELECTION ---------
	const unsigned r1(tournament(size(),true));
	const unsigned r2(tournament(r1,true));

	// --------- CROSSOVER / MUTATION ---------
	individual off;
	if (random::boolean(_env->p_cross))
	{
	  off = _pop[r1].uniform_cross(_pop[r2]);
	  ++stats.crossovers;
	}
	else
	  off = random::boolean() ? _pop[r1] : _pop[r2];

	stats.mutations += off.mutation();

	// --------- REPLACEMENT --------
	const fitness_t f_off(fitness(off));

	const unsigned rep_idx(tournament(r1,false));
	const bool replace(fitness(_pop[rep_idx]) < f_off);

#if !defined(NDEBUG)
	const fitness_t true_f_off(individual::fitness(off)), 
                        true_f_rep(individual::fitness(_pop[rep_idx]));
		
	if (true_f_rep != fitness(_pop[rep_idx]) || 
	    true_f_off != f_off ||
	    stats.f_best != fitness(stats.best))
	  std::cerr << "--> COLLISION <--" << std::endl;
#endif

	if (replace)
	  _pop[rep_idx] = off;

	if (f_off - stats.f_best > float_epsilon)
        {
	  stats.last_imp = stats.gen;
	  stats.best = off;
	  stats.f_best = f_off;

	  if (verbose)
	    std::cout << "Run " << run << '.' << std::setw(6) 
		      << stats.gen << " (" << std::setw(3) 
		      << 100*k/_pop.size() << "%): fitness " << f_off 
		      << std::endl;
        }
      }

      stats.ttable_probes = cache.probes();
      stats.ttable_hits   =   cache.hits();
    }

    if (verbose)
    {
      double speed(0);

      if (clock() > start_c)
	speed = double(_pop.size()*stats.gen)*CLOCKS_PER_SEC/(clock()-start_c);
	
      std::string unit("");
      if (speed >= 10)
	unit = "n/s";
      else if (speed >= 1)
      {
	speed *= 3600;
	unit = "n/h";
      }
      else // speed < 1
      {
	speed *= 3600*24;
	unit = "n/d";
      }

      std::cout << unsigned(speed) << unit << std::string(10,' ') 
		<< std::endl << std::string(40,'-') << std::endl;
    }

    ++run;

    return stats;
  }

  /**
   * fitness
   * \param[in] ind Individual whose fitness we are interested in.
   * \return The fitness of ind.
   *
   * First we try the transposition table. If the information is missing it
   * will be calculated.
   */
  fitness_t
  population::fitness(const individual &ind) const
  {
    fitness_t f;
    if (!cache.find(ind,&f))
    {
      f = individual::fitness(ind);
      cache.insert(ind,f);
    }

    return f;
  }

  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  bool
  population::check() const
  {
    for (unsigned i(0); i < size(); ++i)
      if (!_pop[i].check())
	return false;

    return _env;
  }

  /**
   * operator<<
   * \param[in,out] s
   * \param[in] pop
   * \return
   */
  std::ostream &
  operator<<(std::ostream &s, const population &pop)
  {
    for (population::const_iterator it(pop.begin()); it != pop.end(); ++it)
      s << *it << std::endl;
    
    return s;
  }

  /**
   * clear
   *
   * Resets summary informations.
   */
  void
  summary::clear() 
  { 
    ttable_probes = 0;
    ttable_hits   = 0;
    mutations     = 0;
    crossovers    = 0;
    gen           = 0;
    testset       = 0;
    last_imp      = 0;

    az.clear();
  }

}  // namespace vita
