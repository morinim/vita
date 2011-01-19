/**
 *
 *  \file analyzer.cc
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 */
  
#include "analyzer.h"

namespace vita
{

  /**
   * clear
   *
   * Resets gathered statics.
   */
  void
  analyzer::clear()
  {
    _length.clear();

    _fit.clear();

    _functions[0] = _functions[1] = 0;
    _terminals[0] = _terminals[1] = 0;

    _info.clear();
  }

  /**
   * mutations
   */
  /*
  inline
  boost::uint64_t
  analyzer::mutations() const
  {
    return _pop.stats.mutations;
  }
  */

  /**
   * crossovers
   */
  /*
  inline
  boost::uint64_t
  analyzer::crossovers() const
  {
    return _pop.stats.crossovers;
  }
  */

  /**
   * run
   * \param[in] ind 
   * \return Effective length of individual we gathered statistics about.
   *
   *
   */
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

  /**
   * count
   * \param[in] g
   * \param[in] eff
   */
  void
  analyzer::count(const gene &g, bool eff)
  {
    ++_info[g.sym].counter[eff];

    if (g.sym->argc())
      ++_functions[eff];
    else
      ++_terminals[eff];
  }

  /**
   * add
   * \param[in] ind New individual.
   * \param[in] f Fitness of the new individual.
   *
   * Adds a new individual to the pool used to calculate statistics.
   */
  void
  analyzer::add(const individual &ind, fitness_t f)
  {
    _length.add(run(ind));

    if (!is_bad(f))
      _fit.add(f);
  }
    
  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  bool
  analyzer::check() const
  {
    for (const_iterator i(begin()); i != end(); ++i)
      if (i->second.counter[true] > i->second.counter[false])
	return false;

    return _fit.check() && _length.check();
  }

}  // namespace vita
