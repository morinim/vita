/**
 *
 *  \file data.h
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
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

    data(unsigned = 1);
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
    
    // The training set is partitioned.
    std::vector< std::list<value_type> > _training;

    unsigned _active;
  };

  /**
   * data
   * \param n
   */
  inline
  data::data(unsigned n)
  {
    clear(n);
  }

  /**
   * operator!.
   */
  inline
  bool
  data::operator!() const
  {
    return _training.empty();
  }

  /**
   * begin
   */
  inline
  data::const_iterator
  data::begin() const
  {
    return _training[_active].begin(); 
  }

  /**
   * end
   */
  inline
  data::const_iterator
  data::end() const
  {
    return _training[_active].end(); 
  }

  /**
   * variables
   */
  inline
  unsigned
  data::variables() const
  {
    return _training.empty() || _training[0].empty() 
      ? 0 : _training[0].begin()->input.size(); 
  }

  /**
   * classes
   */
  inline
  unsigned
  data::classes() const
  {
    return _labels.size();
  }
    
}  // namespace vita

#endif  // DATA_H
