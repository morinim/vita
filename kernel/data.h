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
#include <set>
#include <string>
#include <vector>

#include "kernel/distribution.h"

namespace vita
{
  class variable;
  /// Just a shortcut (similar to symbol_ptr).
  typedef std::shared_ptr<variable> variable_ptr;

  ///
  /// \a data class stores the data set used to evolve the vita::population.
  /// It can read xrff (http://weka.wikispaces.com/XRFF) and CSV
  /// (http://code.google.com/intl/it/apis/predict/docs/developer-guide.html)
  /// files.
  ///
  class data
  {
  public:
    ///
    /// \a value_type stores a single element of the data set (instance). The
    /// \c struct consists of an input vector (\a input) and an answer value
    /// (\a output). Depending on the kind of problem, \a output stores:
    /// \li a numeric value (symbolic regression problem);
    /// \li a label (classification problem).
    ///
    /// \a difficulty and \a age are parameters used by the Dynamic Subset
    /// Selection algorithm (see "Dynamic Training Subset Selection for
    /// Supervised Learning in Genetic Programming" - Chris Gathercole, Peter
    /// Ross).
    ///
    struct value_type
    {
      value_type() { clear(); }

      std::vector<boost::any> input;
      boost::any             output;

      boost::uint64_t difficulty;
      unsigned               age;

      unsigned label() const { return boost::any_cast<unsigned>(output); }

      void clear()
      { input.clear(); output = boost::any(); difficulty = 0; age = 0; }
    };

    /// Informations about a "column" of the dataset.
    struct column
    {
      std::string       name;
      category_t category_id;
    };

    ///
    /// \brief Information about a category of the dataset.
    ///
    /// For example:
    /// \verbatim
    ///   <attribute type="nominal">
    ///     <labels>
    ///       <label>Iris-setosa</label>
    ///       <label>Iris-versicolor</label>
    ///       <label>Iris-virginica</label>
    ///     </labels>
    ///   </attribute>
    /// \endverbatim
    /// is mapped to category:
    /// \li {"", d_string, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"}}
    ///
    /// while:
    /// \verbatim <attribute type="numeric" category="A" /> \endverbatim
    /// is mapped to category:
    /// \li  {"A", d_double, {}}
    ///
    struct category
    {
      std::string             name;
      domain_t              domain;
      std::set<std::string> labels;
    };

    /// value_type *
    typedef std::list<value_type>::iterator iterator;
    /// const value_type *
    typedef std::list<value_type>::const_iterator const_iterator;

    data();
    explicit data(const std::string &);

    enum dataset_t {training = 0, validation};
    void dataset(dataset_t);

    const_iterator begin() const;
    const_iterator end() const;
    unsigned size() const;

    unsigned open(const std::string &);
    bool operator!() const;

    void clear();

    category_t get_category(const std::string &) const;
    const category &get_category(category_t) const;
    const column &get_column(unsigned) const;

    unsigned categories() const;
    unsigned classes() const;
    unsigned columns() const;
    unsigned variables() const;

    std::string class_name(unsigned) const;

    bool check() const;

    static const std::map<const std::string, domain_t> from_weka;

  private:
    static boost::any convert(const std::string &, domain_t);
    static unsigned encode(const std::string &,
                           std::map<std::string, unsigned> *);
    static bool is_number(const std::string &);
    static std::vector<std::string> csvline(const std::string &, char = ',',
                                            bool = false);

    unsigned load_csv(const std::string &);
    unsigned load_xrff(const std::string &);

    void swap_category(category_t, category_t);

    // Integer are simpler to manage than textual data, so, when appropriate,
    // input strings are converted into integers by these maps (and the encode
    // static function).
    std::map<std::string, unsigned> categories_map_;
    std::map<std::string, unsigned> classes_map_;

    // How is the dataset organized? Sometimes we have a dataset header (XRFF
    // file format), other times it has to be implicitly derived (e.g. CSV).
    std::vector<column> header_;

    // What are the categories we are dealing with?
    std::vector<category> categories_;

    // Data are stored in two datasets:
    // * a training set used directly for learning;
    // * a validation set for controlling overfitting and measuring the
    //   performance of an individual.
    std::list<value_type> dataset_[2];

    // Used to choose the data we want to operate on (training / validation
    // set).
    // begin(), end() and size() methods operate on the selected set.
    dataset_t active_dataset_;
  };
}  // namespace vita

#endif  // DATA_H
