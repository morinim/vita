/**
 *
 *  \file population.h
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
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

  class population
  {
  public:
    typedef individual value_type;
    typedef std::vector<value_type>::size_type size_type;
    typedef std::vector<value_type>::iterator iterator;
    typedef std::vector<value_type>::const_iterator const_iterator;
    typedef std::vector<value_type>::reference reference;
    typedef std::vector<value_type>::const_reference const_reference;

    individual &operator[](size_type);
    const individual &operator[](size_type) const;
    iterator begin();
    const_iterator begin() const;
    const_iterator end() const;
    size_type size() const;

    explicit population(environment &);
    void build();

    fitness_t fitness(size_type) const;
    fitness_t fitness(const individual &) const;

    unsigned long long probes() const;
    unsigned long long hits() const;

    bool check() const;

    mutable ttable _cache;

  private:
    environment *_env;
    std::vector<value_type> _pop;
  };

  std::ostream &operator<<(std::ostream &, const population &);

  /**
   * Operator[]
   * \param[in] i Index of an individual (interval [0,population size[).
   * \return A reference to the individual at index i.
   */
  inline
  individual &
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
  const individual &
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
   * fitness
   * \param[in] i Inidex of the individual whose fitness we are interested in.
   * \return The fitness of the i-th individual.
   */
  inline
  fitness_t
  population::fitness(size_type i) const
  {
    return fitness(_pop[i]);
  }

  /**
   * probes
   * \return Number of probes in the transposition table.
   */
  inline
  unsigned long long
  population::probes() const
  {
    return _cache.probes();
  }

  /**
   * hits
   * \return Number of transposition table hits.
   */
  inline
  unsigned long long
  population::hits() const
  {
    return _cache.hits();
  }
    
}  // namespace vita

#endif  // POPULATION_H
