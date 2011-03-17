/**
 *
 *  \file analyzer.cc
 *
 *  \author Manlio Morini
 *  \date 2011/03/15
 *
 *  This file is part of VITA
 *
 */
  
#include "analyzer.h"

namespace vita
{

  analyzer::analyzer()
  {
    clear();
  }

  ///
  /// \return a constant reference to the first statistical information.  
  ///
  analyzer::const_iterator
  analyzer::begin() const
  {
    return _info.begin();
  }

  ///
  /// \return a constant reference (sentry) to the last+1 statistical 
  ///         information.
  ///
  analyzer::const_iterator
  analyzer::end() const
  {
    return _info.end();
  }

  ///
  /// \param[in] eff effective / noneffective functions.
  /// \return number of functions in the population.
  ///
  boost::uint64_t
  analyzer::functions(bool eff) const
  {
    return _functions[eff];
  }

  ///
  /// \param[in] eff effective / noneffective terminals.
  /// \return number of terminals in the population.
  ///
  boost::uint64_t
  analyzer::terminals(bool eff) const
  {
    return _terminals[eff];
  }

  ///
  /// \return statistics about the fitness distribution of the individuals.
  ///
  const distribution<fitness_t> &
  analyzer::fit_dist() const
  {
    assert(_fit.check());

    return _fit;
  }

  ///
  /// \return statistic about the length distribution of the individuals.
  ///
  const distribution<double> &
  analyzer::length_dist() const
  {
    assert(_length.check());

    return _length;
  }

  ///
  /// Resets gathered statics.
  ///
  void
  analyzer::clear()
  {
    _length.clear();

    _fit.clear();

    _functions[0] = _functions[1] = 0;
    _terminals[0] = _terminals[1] = 0;

    _info.clear();
  }

  /*
  boost::uint64_t
  analyzer::mutations() const
  {
    return _pop.stats.mutations;
  }
  */

  /*
  boost::uint64_t
  analyzer::crossovers() const
  {
    return _pop.stats.crossovers;
  }
  */

  ///
  /// \param[in] ind individual to be analyzed.
  /// \return effective length of individual we gathered statistics about.
  ///
  unsigned
  analyzer::run(const individual &ind)
  {
    for (unsigned i(0); i < ind.size(); ++i)
      count(ind[i],false);

    unsigned length(0);
    for (individual::const_iterator it(ind); it(); ++it)
    {
      count(*it,true);
      ++length;
    }    

    return length;
  }

  ///
  /// \param[in] g
  /// \param[in] eff
  ///
  void
  analyzer::count(const gene &g, bool eff)
  {
    ++_info[g.sym].counter[eff];

    if (g.sym->argc())
      ++_functions[eff];
    else
      ++_terminals[eff];
  }

  ///
  /// \param[in] ind new individual.
  /// \param[in] f fitness of the new individual.
  ///
  /// Adds a new individual to the pool used to calculate statistics.
  ///
  void
  analyzer::add(const individual &ind, fitness_t f)
  {
    _length.add(run(ind));

    if (!is_bad(f))
      _fit.add(f);
  }
    
  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool
  analyzer::check() const
  {
    for (const_iterator i(begin()); i != end(); ++i)
      if (i->second.counter[true] > i->second.counter[false])
	return false;

    return _fit.check() && _length.check();
  }

}  // namespace vita
