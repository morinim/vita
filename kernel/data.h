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

#include <list>
#include <map>
#include <string>
#include <vector>

#include "kernel/distribution.h"

namespace vita
{
  namespace sr { class variable; }
  typedef std::shared_ptr<sr::variable> variable_ptr;

  ///
  /// \a data class stores the data set used to evolve the vita::population.
  /// It can read xrff (http://weka.wikispaces.com/XRFF) and CSV
  /// (http://code.google.com/intl/it/apis/predict/docs/developer-guide.html)
  /// files.
  ///
  class data
  {
  public:
    /// Categories are defined over some domain.
    enum domain_t {d_void = 0, d_bool, d_double, d_int, d_string};

    ///
    /// \a value_type stores a single element of the data set (instance). The
    /// \c struct consists of an input vector (\a input) and an answer value
    /// (\a output). Depending on the kind of problem, \a output stores:
    /// \li a numeric value (symbolic regression problem);
    /// \li a label (classification problem).
    ///
    struct value_type
    {
      std::vector<boost::any> input;
      boost::any             output;

      unsigned label() const { return boost::any_cast<unsigned>(output); }
      void clear() { input.clear(); output = boost::any(); }
    };

    /// value_type *
    typedef std::list<value_type>::iterator iterator;
    /// const value_type *
    typedef std::list<value_type>::const_iterator const_iterator;

    explicit data(unsigned = 1);
    explicit data(const std::string &, unsigned = 1);

    const_iterator begin() const;
    const_iterator end() const;
    unsigned size() const;

    unsigned open(const std::string &);
    bool operator!() const;

    void clear(unsigned = 1);

    unsigned categories() const;
    unsigned classes() const;
    unsigned variables() const;

    std::string class_name(unsigned) const;

    bool check() const;

  private:
    static boost::any convert(const std::string &, domain_t);
    static unsigned encode(const std::string &,
                           std::map<std::string, unsigned> *);
    static bool is_number(const std::string &);
    static std::vector<std::string> csvline(const std::string &, char = ',',
                                            bool = false);

    unsigned load_csv(const std::string &);
    unsigned load_xrff(const std::string &);

    // Integer are simpler to manage than textual data, so, when appropriate,
    // input strings are converted into integers by these maps (and the encode
    // static function).
    std::map<std::string, unsigned> categories_map_;
    std::map<std::string, unsigned> classes_map_;

    // Informations about a "column" of the dataset.
    struct column
    {
      std::string       name;
      category_t category_id;
      bool            output;
    };

    // How is the dataset organized? Sometimes we have a dataset header (XRFF
    // file format), other times it has to be implicitly derived (e.g. CSV).
    std::vector<column> header_;

    // Information about a category of the dataset. For example:
    //   <attribute type="nominal">
    //     <labels>
    //       <label>Iris-setosa</label>
    //       <label>Iris-versicolor</label>
    //       <label>Iris-virginica</label>
    //     </labels>
    //   </attribute>
    // is mapped to category:
    //   {d_string, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"}}
    // while:
    //   <attribute type="numeric" />
    // is mapped to category:
    //   {d_double, {}}
    struct category
    {
      std::string              name;
      domain_t               domain;
      std::list<boost::any> symbols;
    };

    // What are the categories we are dealing with?
    std::vector<category> categories_;

    // The training set (partitioned).
    std::vector<std::list<value_type>> datasets_;

    // The active data partition.
    // Data are partitioned in multiple datasets:
    // * one or more training sets used directly for learning;
    // * one validation set for controlling overfitting and measuring the
    //   performance of an individual.
    unsigned active_;
  };
}  // namespace vita

#endif  // DATA_H
