/**
 *
 *  \file problem.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(PROBLEM_H)
#define      PROBLEM_H

#include <cmath>

#include "vita.h"
#include "data.h"
#include "environment.h"
#include "primitive/sr_pri.h"

namespace vita
{

  class problem
  {
  public:
    problem();

    bool load_data(const std::string &);
    std::string load_symbols(const std::string &);

    void clear();

    unsigned classes() const;
    unsigned variables() const;

    template<class T>
    fitness_t abs_fitness(const vita::individual &) const;

    template<class T> 
    fitness_t binary_fitness(const vita::individual &, unsigned) const;

    template<class T>
    fitness_t count_fitness(const vita::individual &) const;

    template<class T>
    fitness_t dyn_slot_fitness(const vita::individual &) const;

    bool check() const;

    environment env;
    
  private:
    template<class T> static T normalize_01(T);

    std::vector<vita::sr::variable *> vars;
    data dat;
  };

  ///
  /// \param[in] ind an individual.
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  template<class T>
  fitness_t
  problem::abs_fitness(const vita::individual &ind) const
  {
    assert(!dat.classes());

    interpreter agent(ind);

    fitness_t err(0.0);
    int illegals(0);

    for (data::const_iterator t(dat.begin()); t != dat.end(); ++t)
    {
      for (unsigned i(0); i < vars.size(); ++i)
        vars[i]->val = boost::any_cast<T>(t->input[i]);

      const boost::any res(agent.run());
	
      if (res.empty())
        err += std::pow(100.0,++illegals);
      else     
        err += std::fabs(boost::any_cast<T>(res) - 
                         boost::any_cast<T>(t->output));
    }

    return -err;
  }

  ///
  /// \param[in] ind an individual.
  /// \param[in] label
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  template<class T>
  fitness_t
  problem::binary_fitness(const vita::individual &ind, unsigned label) const
  {
    assert(dat.classes() == 2);

    interpreter agent(ind);

    fitness_t err(0.0);
    int illegals(0);

    for (data::const_iterator t(dat.begin()); t != dat.end(); ++t)
    {
      for (unsigned i(0); i < vars.size(); ++i)
        vars[i]->val = t->input[i];

      const boost::any res(agent.run());

      if (res.empty())
	err += std::pow(100.0,++illegals);
      else
      {
        const T val(boost::any_cast<T>(res));
        if ( (t->label() == label && val < 0.0) ||
	     (t->label() != label && val >= 0.0) )
	  err += std::fabs(val);
      }
    }

    return -err;
  }

  ///
  /// \param[in] ind an individual.
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  /// Slotted Dynamic Class Boundary Determination
  ///
  template<class T>
  fitness_t
  problem::dyn_slot_fitness(const vita::individual &ind) const
  {
    assert(dat.classes() >= 2);

    const unsigned n_slots(dat.classes()*10);
    std::vector< std::vector<unsigned> > slots(n_slots);
    for (unsigned i(0); i < slots.size(); ++i)
    {
      slots[i].resize(dat.classes());

      for (unsigned j(0); j < slots[i].size(); ++j)
        slots[i][j] = 0;
    }

    interpreter agent(ind);

    // In the first step this method evaluates the program to obtain an output 
    // value for each training example. Based on the program output value a
    // a bidimentional array is built (slots[slot][class]).
    for (data::const_iterator t(dat.begin()); t != dat.end(); ++t)
    {
      for (unsigned i(0); i < vars.size(); ++i)
        vars[i]->val = boost::any_cast<T>(t->input[i]);
      
      const boost::any res(agent.run());

      unsigned slot;
      if (res.empty())
        slot = slots.size()-1;
      else
      {
        const T val(boost::any_cast<T>(res));
        slot = static_cast<unsigned>(normalize_01<T>(val) * n_slots);
        if (slot >= slots.size())
          slot = slots.size()-1;
      }

      ++slots[slot][t->label()];
    }

    std::vector<unsigned> slot_label(n_slots);
    const unsigned unknown(dat.classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the 
    // slot...
    for (unsigned i(0); i < n_slots; ++i)
    {
      unsigned best_class(0);

      for (unsigned j(1); j < slots[i].size(); ++j)
	if (slots[i][j] >= slots[i][best_class])
	  best_class = j;

      slot_label[i] = slots[i][best_class] ? best_class : unknown;
    }

    fitness_t err(0.0);
    for (unsigned i(0); i < n_slots; ++i)
      for (unsigned j(0); j < slots[i].size(); ++j)
	if (j != slot_label[i])
	  err += slots[i][j];

    return -err;
  }

  ///
  /// \param[in] ind an individual.
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  template<class T>
  fitness_t
  problem::count_fitness(const vita::individual &ind) const
  {
    assert(!dat.classes());

    interpreter agent(ind);

    unsigned ok(0);

    for (data::const_iterator t(dat.begin()); t != dat.end(); ++t)
    {
      for (unsigned i(0); i < vars.size(); ++i)
	vars[i]->val = t->input[i];

      const boost::any res(agent.run());
	
      if (!res.empty() && boost::any_cast<T>(res) == t->output)
	++ok;
    }

    return fitness_t(ok);
  }

  ///
  /// \param[in] val the numeric value that should be mapped in the [0,1]
  ///                interval.
  ///
  template<class T>
  inline
  T
  problem::normalize_01(T val)
  {
    return 0.5 + std::atan(val)/3.1415926535;
  }

}  // namespace vita

#endif  // PROBLEM_H
