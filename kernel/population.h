/**
 *
 *  \file population.h
 *
 *  \author Manlio Morini
 *  \date 2011/04/11
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(POPULATION_H)
#define      POPULATION_H

#include "vita.h"
#include "analyzer.h"
#include "individual.h"

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

    const environment &env() const;

    bool check() const;

  private:
    environment *_env;
    std::vector<value_type> _pop;
  };

  std::ostream &operator<<(std::ostream &, const population &);

  ///
  /// \param[in] i index of an individual (in the [0,population size[ range).
  /// \return a reference to the individual at index \a i.
  ///
  inline
  individual &
  population::operator[](size_type i)
  {
    assert(i < _pop.size());
    return _pop[i];
  }

  ///
  /// \param[in] i index of an individual (int the [0,population size[ range).
  /// \return a constant reference to the individual at index \a i.
  ///
  inline
  const individual &
  population::operator[](size_type i) const
  { 
    assert(i < _pop.size());
    return _pop[i];  
  }

  ///
  /// \return a reference to the first individual of the population.
  ///
  inline
  population::iterator
  population::begin()
  {
    return _pop.begin();
  }

  ///
  /// \return a constant reference to the first individual of the population.
  ///
  inline
  population::const_iterator 
  population::begin() const
  {
    return _pop.begin();
  }

  ///
  /// \return a constant reference to the last+1 individual of the population.
  ///
  inline
  population::const_iterator 
  population::end() const
  {
    return _pop.end();
  }

  ///
  /// \return the number of individuals in the population.
  ///
  inline
  population::size_type
  population::size() const
  {
    return _pop.size();
  }

  ///
  /// \example example2.cc
  /// Creates a random population and shows its content.
  ///
    
}  // namespace vita

#endif  // POPULATION_H
