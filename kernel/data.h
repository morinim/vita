/**
 *
 *  \file data.h
 *
 *  \author Manlio Morini
 *  \date 2010/12/29
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(DATA_H)
#define      DATA_H

#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/any.hpp>

#include "vita.h"
#include "distribution.h"

namespace vita
{

  ///
  /// \a data \c class stores the training set used to evolve the population.
  ///
  /// \todo The class should also contains a validation/test set to avoid
  ///       overfitting.
  /// 
  ///
  class data
  {
  public:
    ///
    /// \a value_type stores a single element of the training set. The \c struct
    /// consists of an input vector (\a input) and an answer value (\a output).
    /// Depending on the kind of problem, \a output stores:
    /// \li a numeric value (symbolic regression problem);
    /// \li a label (classification problem).
    ///
    struct value_type
    { 
      std::vector<boost::any> input;
      boost::any             output;

      unsigned label() const { return boost::any_cast<unsigned>(output); };
    };

    typedef std::list<value_type>::iterator iterator;
    typedef std::list<value_type>::const_iterator const_iterator;
    typedef std::list<value_type>::reference reference;
    typedef std::list<value_type>::const_reference const_reference;

    const_iterator begin() const;
    const_iterator end() const;

    explicit data(unsigned = 1);
    explicit data(const std::string &, unsigned = 1);

    unsigned open(const std::string &);

    bool operator!() const;

    void clear(unsigned = 1);

    unsigned variables() const;
    unsigned classes() const;
  
    bool check() const;

  private:
    unsigned encode(const std::string &);

    std::map<std::string,unsigned> _labels;
    
    /// The training set (partitioned).
    std::vector< std::list<value_type> > _training;

    /// The active data partition.
    unsigned _active;
  };

  ///
  /// \return constant reference to the first element of the active training
  ///         set.
  ///
  inline
  data::const_iterator
  data::begin() const
  {
    return _training[_active].begin(); 
  }

  ///
  /// \return constant reference to the last+1 element of the active training
  ///         set.
  ///
  inline
  data::const_iterator
  data::end() const
  {
    return _training[_active].end(); 
  }
   
}  // namespace vita

#endif  // DATA_H
