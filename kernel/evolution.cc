/**
 *
 *  \file evolution.cc
 *
 *  \author Manlio Morini
 *  \date 2011/04/13
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
  /// \param[in] pop the \ref population that will be evolved. 
  /// \param[in] eva evaluator used during the evolution.
  ///
  evolution::evolution(vita::population &pop, evaluator *const eva) 
    : selection(this), operation(this), replacement(this), _pop(pop), 
      _eva(new evaluator_proxy(eva,pop.env().ttable_size)) 
  {
    assert(eva);

    _run_count = 0;

    assert(check());
  }

  ///
  evolution::~evolution()
  {
    delete _eva;
  }

  ///
  /// \return access to the population being evolved.
  ///
  vita::population &
  evolution::population() const
  {
    return _pop;
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

    for (population::const_iterator i(_pop.begin()); i != _pop.end(); ++i) 
      az->add(*i,_eva->run(*i));
  }

  void
  evolution::pick_stats()
  {
    pick_stats(&_stats.az);
  }

  ///
  /// Saves working / statistical informations in a log file.
  /// Data are written in a CSV-like fashion and are partitioned in blocks
  /// separated by two blank lines:
  /// [BLOCK_1]\n\n
  /// [BLOCK_2]\n\n 
  /// ...
  /// [BLOCK_x]
  /// where each block is a set of line like this:
  /// data_1 [space] data_2 [space] ... [space] data_n
  /// We use this format, instead of XML, because statistics are produced
  /// incrementally and so it's simple and fast to append new data to a 
  /// CSV-like file. Note also that data sets are ready to be plotted by 
  /// GNUPlot.
  /// 
  void
  evolution::log() const
  {
    static unsigned last_run(0);

    if (_pop.env().stat_dynamic)
    {
      const std::string f_dynamic(_pop.env().stat_dir + "/dynamic");
      std::ofstream dynamic(f_dynamic.c_str(),std::ios_base::app);
      if (dynamic.good())
      {
        if (last_run != _run_count)
        {
          dynamic << std::endl << std::endl;
          last_run = _run_count;
        }
        
        dynamic << _run_count << ' ' << _stats.gen
                << ' ' << _stats.f_best << ' ' << _stats.az.fit_dist().mean 
                << ' ' << _stats.az.fit_dist().min
                << ' ' << _stats.az.fit_dist().standard_deviation()
                << ' ' << unsigned(_stats.az.length_dist().mean) 
                << ' ' << _stats.az.length_dist().standard_deviation()
                << ' ' << unsigned(_stats.az.length_dist().max)
                << ' ' << _stats.mutations << ' ' << _stats.crossovers
                << ' ' << _stats.az.functions(0) 
                << ' ' << _stats.az.terminals(0)
                << ' ' << _stats.az.functions(1) 
                << ' ' << _stats.az.terminals(1)
                << ' ' << _eva->hits()
                << ' ' << _eva->probes();
        
        for (unsigned active(0); active <= 1; ++active)
          for (analyzer::const_iterator i(_stats.az.begin());
               i != _stats.az.end();
               ++i)
            dynamic << ' ' << (i->first)->display() << ' ' 
                    << i->second.counter[active];

        dynamic << " \"";
        _stats.best.inline_tree(dynamic);
        dynamic << '"' << std::endl;
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
      (_pop.env().g_since_start && _stats.gen > _pop.env().g_since_start) ||

      // We use an accelerated stop condition when all the individuals have
      // the same fitness and after gwi/2 generations the situation isn't
      // changed. 
      ( _pop.env().g_without_improvement && 
        (_stats.gen-_stats.last_imp > _pop.env().g_without_improvement ||
	 (_stats.gen-_stats.last_imp > _pop.env().g_without_improvement/2 &&
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
  /// \param[in] sel_id index of the active selection strategy.
  /// \param[in] op_id index of the active operation strategy.
  /// \param[in] rep_id index of the active replacement strategy.
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
  evolution::run(bool verbose, unsigned sel_id, unsigned op_id, unsigned rep_id)
  {
    _stats.clear();
    _stats.best   = *_pop.begin();
    _stats.f_best = _eva->run(_stats.best);

    _eva->clear();

    std::clock_t start_c(clock());
    for (_stats.gen = 0; !stop_condition(); ++_stats.gen)
    {
      pick_stats(&_stats.az);

      log();

      for (unsigned k(0); k < _pop.size(); ++k)
      {
	if ( verbose && k % std::max(_pop.size()/100,size_t(1)) )
	{
	  std::cout << "Run " << _run_count << '.'
		    << _stats.gen << " (" << std::setw(3) << 100*k/_pop.size() 
		    << "%)\r" << std::flush;
	}

	// --------- SELECTION ---------
        std::vector<unsigned> parents(selection.get(sel_id)->run());

	// --------- CROSSOVER / MUTATION ---------
        std::vector<individual> off(operation.get(op_id)->run(parents,&_stats));

	// --------- REPLACEMENT --------
        const fitness_t before(_stats.f_best);
        replacement.get(rep_id)->run(parents,off,&_stats);

        if (verbose && _stats.f_best != before)
          std::cout << "Run " << _run_count << '.' << std::setw(6) 
                    << _stats.gen << " (" << std::setw(3) 
                    << 100*k/_pop.size() << "%): fitness " << _stats.f_best 
                    << std::endl;
      }

      _stats.ttable_probes = _eva->probes();
      _stats.ttable_hits   =   _eva->hits();
    }

    if (verbose)
    {
      double speed(0);

      if (clock() > start_c)
	speed = double(_pop.size()*_stats.gen)*CLOCKS_PER_SEC/(clock()-start_c);
	
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
    return _pop.check();
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
