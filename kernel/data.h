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

  class data
  {
  public:
    struct value_type
    { 
      std::vector<boost::any> input;
      boost::any             output;
      unsigned                label;
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

    /// This is the active data partition.
    unsigned _active;
  };

  ///
  /// \return Constant reference to the first element of the active training
  //          set.
  ///
  inline
  data::const_iterator
  data::begin() const
  {
    return _training[_active].begin(); 
  }

  ///
  /// \return Constant reference to the last+1 element of the active training
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
