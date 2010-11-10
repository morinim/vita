/**
 *
 *  \file population.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/10
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(POPULATION_H)
#define      POPULATION_H

#include "vita.h"
#include "analyzer.h"
#include "individual.h"
#include "ttable.h"

namespace vita
{

  class environment;

  struct summary
  {
    summary() { clear(); };

    void clear();

    unsigned long long ttable_probes;
    unsigned long long   ttable_hits;

    unsigned long long  mutations;
    unsigned long long crossovers;

    unsigned gen;
    unsigned testset;
    unsigned last_imp;

    analyzer az;

    individual  best;
    fitness_t f_best;
  };

  class population
  {
  public:
    typedef individual value_type;
    typedef std::vector<value_type>::size_type size_type;
    typedef std::vector<value_type>::iterator iterator;
    typedef std::vector<value_type>::const_iterator const_iterator;
    typedef std::vector<value_type>::reference reference;
    typedef std::vector<value_type>::const_reference const_reference;

    reference operator[](size_type);
    const_reference operator[](size_type) const;
    iterator begin();
    const_iterator begin() const;
    const_iterator end() const;
    size_type size() const;

    explicit population(environment &);
    void build();

    void pick_stats(analyzer *const);

    fitness_t fitness(const individual &) const;
    const summary &evolution(bool);

    bool check() const;

  private:
    static unsigned run;

    void log() const;
    void pick_stats();

    bool stop_condition() const;
    unsigned tournament(unsigned,bool) const;

    environment *_env;

    std::vector<value_type> _pop;

    mutable ttable cache;

    summary stats;
  };

  std::ostream &operator<<(std::ostream &, const population &);

  /**
   * Operator[]
   * \param[in] i Index of an individual (interval [0,population size[).
   * \return A reference to the individual at index i.
   */
  inline
  population::reference
  population::operator[](size_type i)
  {
    assert(i < _pop.size());
    return _pop[i];
  }

  /**
   * Operator[]
   * \param[in] i Index of an individual (interval [0,population size[).
   * \return A constant reference to the individual at index i.
   */
  inline
  population::const_reference
  population::operator[](size_type i) const
  { 
    assert(i < _pop.size());
    return _pop[i];  
  }

  /**
   * begin
   * \return A reference to the first individual of the population.
   */
  inline
  population::iterator
  population::begin()
  {
    return _pop.begin();
  }

  /**
   * begin
   * \return A constant reference to the first individual of the population.
   */
  inline
  population::const_iterator 
  population::begin() const
  {
    return _pop.begin();
  }

  /**
   * end
   * \return A constant reference to the last+1 individual of the population.
   */
  inline
  population::const_iterator 
  population::end() const
  {
    return _pop.end();
  }

  /**
   * size
   * \return The number of individuals of the population.
   */
  inline
  population::size_type
  population::size() const
  {
    return _pop.size();
  }

  /**
   * pick_stats
   */
  inline
  void
  population::pick_stats()
  {
    pick_stats(&stats.az);
  }
    
}  // namespace vita

#endif  // POPULATION_H
