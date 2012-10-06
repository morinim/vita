/**
 *
 *  \file data.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(DATA_H)
#define      DATA_H

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/variant.hpp>

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
  public:  // Structures.
    ///
    /// \a example stores a single element of the data set. The
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
    struct example
    {
      typedef boost::variant<bool, int, double, std::string> value_t;

      example() { clear(); }

      std::vector<value_t> input;
      value_t             output;

      boost::uint64_t difficulty;
      unsigned               age;

      unsigned label() const
      { return static_cast<unsigned>(boost::get<int>(output)); }

      void clear()
      { input.clear(); output = value_t(); difficulty = 0; age = 0; }
    };

    template<class T> static T cast(const example::value_t &);

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
    ///
    ///     <attribute type="nominal">
    ///       <labels>
    ///         <label>Iris-setosa</label>
    ///         <label>Iris-versicolor</label>
    ///         <label>Iris-virginica</label>
    ///       </labels>
    ///     </attribute>
    ///
    /// is mapped to category:
    /// \li {"", d_string, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"}}
    ///
    /// while:
    ///     <attribute type="numeric" category="A" />
    /// is mapped to category:
    /// \li  {"A", d_double, {}}
    ///
    struct category
    {
      std::string             name;
      domain_t              domain;
      std::set<std::string> labels;
    };

  public:
    /// example *
    typedef std::list<example>::iterator iterator;
    /// const example *
    typedef std::list<example>::const_iterator const_iterator;

  public:  // Construction, convenience.
    data();
    explicit data(const std::string &);

    enum dataset_t {training = 0, validation};
    void dataset(dataset_t, unsigned = 0);

    iterator begin();
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    unsigned size() const;

    unsigned open(const std::string &);
    bool operator!() const;

    void clear();

    void sort(std::function<bool (const example &, const example &)>);

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
    static example::value_t convert(const std::string &, domain_t);
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
    std::list<example> dataset_[2];

    // Used to choose the data we want to operate on (training / validation
    // set).
    // begin(), end() and size() methods operate on the selected set.
    dataset_t active_dataset_;

    // Used to select a subset of the active dataset.
    data::const_iterator end_;
  };

  template<class T>
  T data::cast(const example::value_t &e)
  {
    switch (e.which())
    {
    case 0:  return static_cast<T>(boost::get<bool>(e));
    case 1:  return static_cast<T>(boost::get<int>(e));
    case 2:  return static_cast<T>(boost::get<double>(e));
    default: return static_cast<T>(0.0);
    }
  }
}  // namespace vita

#endif  // DATA_H
