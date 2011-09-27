/**
 *
 *  \file data.h
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(DATA_H)
#define      DATA_H

#include <boost/any.hpp>

#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "kernel/distribution.h"

namespace vita
{
  namespace sr { class variable; }
  typedef std::shared_ptr<sr::variable> variable_ptr;

  ///
  /// \a data \c class stores the training set used to evolve the \a population.
  ///
  /// \todo The \c class should also contains a validation/test set to avoid
  ///       overfitting.
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

      unsigned label() const { return boost::any_cast<unsigned>(output); }
    };

    typedef std::list<value_type>::iterator iterator;
    typedef std::list<value_type>::const_iterator const_iterator;
    typedef std::list<value_type>::reference reference;
    typedef std::list<value_type>::const_reference const_reference;

    explicit data(unsigned = 1);
    explicit data(const std::string &, unsigned = 1);

    const_iterator begin() const;
    const_iterator end() const;

    unsigned open(const std::string &);
    bool operator!() const;

    void clear(unsigned = 1);

    unsigned variables() const;
    unsigned classes() const;

    bool check() const;

  private:
    unsigned encode(const std::string &);

    std::map<std::string, unsigned> labels_;

    /// The training set (partitioned).
    std::vector< std::list<value_type> > training_;

    /// The active data partition.
    unsigned active_;
  };
}  // namespace vita

#endif  // DATA_H
