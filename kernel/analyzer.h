/**
 *
 *  \file analyzer.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/10
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(ANALYZER_H)
#define      ANALYZER_H

#include <list>
#include <map>
#include <set>

#include "vita.h"
#include "distribution.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{
  struct stats
  {
    unsigned counter[2];
  };

  class analyzer
  {
  public:
    typedef std::map<const symbol *,stats>::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;
    
    analyzer();

    unsigned run(const individual &);
    void add(const individual &, fitness_t);

    void clear();

    unsigned long long functions(bool) const;
    unsigned long long terminals(bool) const;

    const distribution<fitness_t> &fit_dist() const;
    const distribution<double> &length_dist() const;

    unsigned long long mutations() const;
    unsigned long long crossovers() const;

    bool check() const;

  private:
    void count(const gene &, bool);

    std::map<const symbol *, stats> _info;

    distribution<fitness_t> _fit;
    distribution<double> _length;

    unsigned long long _functions[2];
    unsigned long long _terminals[2];
  };

  /**
   * analyzer
   */
  inline
  analyzer::analyzer()
  {
    clear();
  }

  /**
   * begin
   * \return A constant reference to the first statistical information.  
   */
  inline
  analyzer::const_iterator
  analyzer::begin() const
  {
    return _info.begin();
  }

  /**
   * end
   * \return A constant reference to the last+1 statistical information.  
   */
  inline
  analyzer::const_iterator
  analyzer::end() const
  {
    return _info.end();
  }

  /**
   * functions
   * \param[in] eff Effective / noneffective functions.
   * \return Number of functions in the population.
   */
  inline
  unsigned long long
  analyzer::functions(bool eff) const
  {
    return _functions[eff];
  }

  /**
   * terminals
   * \param[in] eff Effective / noneffective terminals.
   * \return Number of terminals in the population.
   */
  inline
  unsigned long long
  analyzer::terminals(bool eff) const
  {
    return _terminals[eff];
  }

  /**
   * fit_dist
   * \return Statistics about the fitness distribution of the individuals.
   */
  inline
  const distribution<fitness_t> &
  analyzer::fit_dist() const
  {
    assert(_fit.check());

    return _fit;
  }

  /**
   * length_dist
   * \return Statistic about the length distribution of the individuals.
   */
  inline
  const distribution<double> &
  analyzer::length_dist() const
  {
    assert(_length.check());

    return _length;
  }

}  // namespace vita

#endif  // ANALYZER_H
