/**
 *
 *  \file evolution.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
 *
 */
  
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "evolution.h"
#include "environment.h"
#include "random.h"

namespace vita
{

  ///
  /// \param[in] e base environment.
  /// \param[in] eva evaluator used during the evolution.
  ///
  evolution::evolution(environment &e, evaluator *const eva) 
    : _env(&e), _pop(new vita::population(e)), 
      _eva(new evaluator_proxy(eva,e.ttable_size)) 
  {
    assert(e.check());

    _run_count = 0;

    assert(check());
  }

  ///
  evolution::~evolution()
  {
    delete _pop;
    delete _eva;
  }

  ///
  /// \return read-only access to the population being evolved.
  ///
  const vita::population &
  evolution::population() const
  {
    return *_pop;
  }

  ///
  /// \param[out] az repository for the statistical informations.
  ///
  /// Gathers statistical informations about the elements of the population.
  ///
  void
  evolution::pick_stats(analyzer *const az)
  {
    az->clear();

    for (population::const_iterator i(_pop->begin()); i != _pop->end(); ++i)    
      az->add(*i,_eva->run(*i));
  }

  void
  evolution::pick_stats()
  {
    pick_stats(&_stats.az);
  }

  ///
  /// \param[in] target index of an individual in the population.
  /// \param[in] best are we looking for the best (or the worst) individual?
  /// \return index of the best (worst) individual found.
  ///
  /// Tournament selection works by selecting a number of individuals from the 
  /// population at random, a tournament, and then selecting only the best of 
  /// those individuals.
  /// Recall that better individuals have highter fitnesses.
  ///
  unsigned
  evolution::tournament(unsigned target, bool best) const
  {
    const unsigned n(_pop->size());
    const unsigned mate_zone(_env->mate_zone);
    const unsigned rounds(best ? _env->par_tournament : _env->rep_tournament);

    unsigned sel(random::ring(target,mate_zone,n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const unsigned j(random::ring(target,mate_zone,n));

      const fitness_t fit_j(_eva->run((*_pop)[j]));
      const fitness_t fit_sel(_eva->run((*_pop)[sel]));
      if (best)
      {
        if (fit_j > fit_sel)
          sel = j;
      }
      else  // worst.
      {
        if (fit_j < fit_sel)
	  sel = j;
      }
    }

    return sel;
  }

  ///
  /// Saves working / statistical informations in the log files.
  ///
  void
  evolution::log() const
  {
    if ( (_env->stat_period && _stats.gen % _env->stat_period == 0) || 
	 _env->stat_dynamic)
    {
      if (_env->stat_period)
      { 
	std::ostringstream f_logs;
	f_logs << _env->stat_dir << '/' << _run_count << '_' << _stats.gen;
	std::ofstream logs(f_logs.str().c_str());

	if (logs.good())
	{
	  logs << "{Best individual (fitness)}" << std::endl
	       << _stats.f_best << std::endl;

	  logs << std::endl << "{Best individual (list)}" << std::endl;
	  _stats.best.list(logs);
	  logs << std::endl <<"{Best individual (tree)}" << std::endl;
	  _stats.best.tree(logs);
	  logs << std::endl <<"{Best individual (graphviz)}" << std::endl;
	  _stats.best.graphviz(logs);

	  for (unsigned active(0); active <= 1; ++active)
	  {
	    const boost::uint64_t nf(_stats.az.functions(active));
	    const boost::uint64_t nt(_stats.az.terminals(active));
	    const boost::uint64_t n(nf+nt);
	    const std::string s(active ? "Active" : "Overall");
	    
	    logs << std::endl << '{' << s << " symbol frequency}" 
		 << std::endl;
	    for (analyzer::const_iterator i(_stats.az.begin());
		 i != _stats.az.end();
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
	       << _stats.mutations << std::endl
	       << std::endl << "{Crossovers}" << std::endl
	       << _stats.crossovers 
	       << std::endl << std::endl 
	       << "{Average fitness}" << std::endl
	       << _stats.az.fit_dist().mean << std::endl
	       << std::endl << "{Worst fitness}" << std::endl
	       << _stats.az.fit_dist().min << std::endl
	       << std::endl << "{Fitness standard deviation}" << std::endl
	       << _stats.az.fit_dist().standard_deviation() 
	       << std::endl << std::endl 
	       << "{Average code length}" << std::endl
	       << unsigned(_stats.az.length_dist().mean) << std::endl
	       << std::endl << "{Lenth standard deviation}" << std::endl
	       << std::sqrt(_stats.az.length_dist().variance) << std::endl
	       << std::endl << "{Max code length}" << std::endl
	       << unsigned(_stats.az.length_dist().max) 
	       << std::endl << std::endl 
	       << "{Hit rate}" << std::endl
	       << (_eva->probes() ? _eva->hits()*100/_eva->probes() : 0)
	       << std::endl;
	}
      }

      if (_env->stat_dynamic)
      {
	const std::string f_dynamic(_env->stat_dir + "/dynamic");
	std::ofstream dynamic(f_dynamic.c_str(),std::ios_base::app);
	if (dynamic.good())
	  dynamic << _run_count << ' ' << _stats.gen
		  << ' ' << _stats.f_best << ' ' << _stats.az.fit_dist().mean 
		  << ' ' << _stats.az.fit_dist().min
		  << ' ' << _stats.az.fit_dist().standard_deviation()
		  << ' ' << unsigned(_stats.az.length_dist().mean) 
		  << ' ' << std::sqrt(_stats.az.length_dist().variance)
		  << ' ' << unsigned(_stats.az.length_dist().max)
		  << ' ' << _stats.mutations << ' ' << _stats.crossovers
		  << ' ' << _stats.az.functions(0) 
		  << ' ' << _stats.az.terminals(0)
		  << ' ' << _stats.az.functions(1) 
		  << ' ' << _stats.az.terminals(1)
		  << ' ' << (_eva->probes() ? _eva->hits()*100/_eva->probes()
			                    : 0)
		  << std::endl;
      }
    }
  }

  ///
  /// \return true if evolution should be interrupted.
  ///
  bool
  evolution::stop_condition() const
  {
    return 
      (_env->g_since_start && _stats.gen > _env->g_since_start) ||

      // We use an accelerated stop condition when all the individuals have
      // the same fitness and after gwi/2 generations the situation isn't
      // changed. 
      ( _env->g_without_improvement && 
        (_stats.gen-_stats.last_imp > _env->g_without_improvement ||
	 (_stats.gen-_stats.last_imp > _env->g_without_improvement/2 &&
         _stats.az.fit_dist().variance <= float_epsilon)) );
  }

  ///
  /// \param[in] ind individual whose fitness we are interested in.
  /// \return the fitness of \a ind.
  ///
  fitness_t
  evolution::fitness(const individual &ind) const
  {
    return _eva->run(ind);
  }

  ///
  /// \param[in] verbose if true prints verbose informations.
  ///
  /// the genetic programming loop. We begin the loop by choosing a
  /// genetic operation: reproduction, mutation or crossover. We then select 
  /// the individual(s) to participate in the genetic operation using 
  /// tournament selection. If we are doing reproduction or mutation, we only 
  /// select one individual. For crossover, two individuals need to be selected.
  /// The genetic operation is then performed and a new offspring individual is
  /// created.
  /// The offspring is then placed into the original population (steady state)
  /// replacing a bad individual. 
  /// This whole process repeats until the termination criteria is satisfied.
  /// With any luck, this process will produce an individual that solves the 
  /// problem at hand. 
  ///
  const summary &
  evolution::run(bool verbose)
  {
    _stats.clear();
    _stats.best   = *_pop->begin();
    _stats.f_best = _eva->run(_stats.best);

    _eva->clear();

    std::clock_t start_c(clock());
    for (_stats.gen = 0; !stop_condition(); ++_stats.gen)
    {
      pick_stats(&_stats.az);

      log();

      for (unsigned k(0); k < _pop->size(); ++k)
      {
	if ( verbose && k % std::max(_pop->size()/100,size_t(1)) )
	{
	  std::cout << "Run " << _run_count << '.'
		    << _stats.gen << " (" << std::setw(3) << 100*k/_pop->size() 
		    << "%)\r" << std::flush;
	}

	// --------- SELECTION ---------
	const unsigned r1(tournament(_pop->size(),true));
	const unsigned r2(tournament(r1,true));

	// --------- CROSSOVER / MUTATION ---------
	individual off;
	if (random::boolean(_env->p_cross))
	{
	  off = (*_pop)[r1].uniform_cross((*_pop)[r2]);
	  ++_stats.crossovers;
	}
	else
	  off = random::boolean() ? (*_pop)[r1] : (*_pop)[r2];

	_stats.mutations += off.mutation();

	// --------- REPLACEMENT --------
	const fitness_t f_off(_eva->run(off));

	const unsigned rep_idx(tournament(r1,false));
	const fitness_t f_rep_idx(_eva->run((*_pop)[rep_idx]));
	const bool replace(f_rep_idx < f_off);

	if (replace)
	  (*_pop)[rep_idx] = off;

	if (f_off - _stats.f_best > float_epsilon)
        {
	  _stats.last_imp = _stats.gen;
	  _stats.best = off;
	  _stats.f_best = f_off;

	  if (verbose)
	    std::cout << "Run " << _run_count << '.' << std::setw(6) 
		      << _stats.gen << " (" << std::setw(3) 
		      << 100*k/_pop->size() << "%): fitness " << f_off 
		      << std::endl;
        }
      }

      _stats.ttable_probes = _eva->probes();
      _stats.ttable_hits   =   _eva->hits();
    }

    if (verbose)
    {
      double speed(0);

      if (clock() > start_c)
	speed = double(_pop->size()*_stats.gen)*CLOCKS_PER_SEC/(clock()-start_c);
	
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

    ++_run_count;

    return _stats;
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool
  evolution::check() const
  {
    return _env && _pop->check();
  }

  ///
  /// Resets summary informations.
  ///
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
