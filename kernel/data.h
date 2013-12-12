/**
 *
 *  \file data.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
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

#include "kernel/any.h"
#include "kernel/distribution.h"

namespace vita
{
  ///
  /// Stores the dataset used to evolve vita::population.
  /// It can read xrff (http://weka.wikispaces.com/XRFF) and CSV
  /// (https://developers.google.com/prediction/docs/developer-guide?hl=it)
  /// files.
  ///
  class data
  {
  public:  // Structures.
    ///
    /// \a example stores a single element of the data set. The
    /// \c struct consists of an input vector (\a input) and an answer value
    /// (\a output). Depending on the kind of problem, \a output stores:
    /// * a numeric value (symbolic regression problem);
    /// * a label (classification problem).
    ///
    /// \a difficulty and \a age are parameters used by the Dynamic Subset
    /// Selection algorithm (see "Dynamic Training Subset Selection for
    /// Supervised Learning in Genetic Programming" - Chris Gathercole, Peter
    /// Ross).
    ///
    struct example
    {
      example() { clear(); }

      std::vector<any> input;
      any             output;
      domain_t      d_output;

      std::uintmax_t  difficulty;
      unsigned               age;

      unsigned label() const { return any_cast<unsigned>(output); }
      template<class T> T cast_output() const;
    
      void clear()
      {
        input.clear();
        output = any();
        d_output = d_void;
        difficulty = 0;
        age = 0;
      }
    };

    /// \brief Informations about a "column" (feature) of the dataset.
    struct column
    {
      std::string       name;
      category_t category_id;
    };

    ///
    /// \brief Informations about a category of the dataset.
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
    /// * {"", d_string, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"}}
    ///
    /// while:
    ///     <attribute type="numeric" category="A" name="Speed" />
    /// is mapped to category:
    /// * {"A", d_double, {}}
    ///
    struct category
    {
      std::string             name;
      domain_t              domain;
      std::set<std::string> labels;
    };

  public:
    /// example *
    typedef typename std::list<example>::iterator iterator;
    /// const example *
    typedef typename std::list<example>::const_iterator const_iterator;

  public:  // Construction, convenience.
    data();
    explicit data(const std::string &, unsigned = 0);

    enum dataset_t {training = 0, validation, test, k_max_dataset = test};
    void dataset(dataset_t);
    dataset_t dataset() const;
    void slice(unsigned);

    iterator begin();
    const_iterator cbegin() const;
    iterator end() const;
    const_iterator cend() const { return end(); }
    unsigned size() const;
    unsigned size(dataset_t) const;

    unsigned open(const std::string &, unsigned = 0);
    bool operator!() const;

    void clear();

    void divide(double);
    void sort(std::function<bool (const example &, const example &)>);

    category_t get_category(const std::string &) const;
    const category &get_category(category_t) const;
    const column &get_column(unsigned) const;

    unsigned categories() const;
    unsigned classes() const;
    unsigned columns() const;
    unsigned variables() const;

    std::string class_name(unsigned) const;

    bool debug() const;

    static domain_t from_weka(const std::string &n)
    {
      static const std::map<const std::string, domain_t> map(
      {
        // This type is vita-specific (not standard).
        {"boolean", domain_t::d_bool},

        {"integer", domain_t::d_int},

        // Real and numeric are treated as double precisione number (d_double).
        {"numeric", domain_t::d_double},
        {"real", domain_t::d_double},

        // Nominal values are defined by providing a list of possible values.
        {"nominal", domain_t::d_string},

        // String attributes allow us to create attributes containing arbitrary
        // textual values. This is very useful in text-mining applications.
        {"string", domain_t::d_string}

        // {"date", ?}, {"relational", ?}
      });

      const auto &i(map.find(n));
      return i == map.end() ? d_void : i->second;
    }
    
  private:
    static unsigned encode(const std::string &,
                           std::map<std::string, unsigned> *);
    static bool is_number(const std::string &);
    static std::vector<std::string> csvline(const std::string &, char = ',',
                                            bool = false);

    unsigned load_csv(const std::string &, unsigned);
    unsigned load_xrff(const std::string &);

    void swap_category(category_t, category_t);

  private:  // Private data members.
    /// Integer are simpler to manage than textual data, so, when appropriate,
    /// input strings are converted into integers by these maps (and the encode
    /// static function).
    std::map<std::string, unsigned> categories_map_;
    std::map<std::string, unsigned> classes_map_;

    /// How is the dataset organized? Sometimes we have a dataset header (XRFF
    /// file format), other times it has to be implicitly derived (e.g. CSV).
    std::vector<column> header_;

    /// What are the categories we are dealing with?
    std::vector<category> categories_;

    /// Data are stored in three datasets:
    /// * a training set used directly for learning;
    /// * a validation set for controlling overfitting and measuring the
    ///   performance of an individual;
    /// * a test set for a forecast of how well an individual will do in the
    ///   real world.
    /// We don't validate on the training data because that would overfit the
    /// model. We don't stop at the validation step because we've iteratively
    /// been adjusting things to get a winner in the validation step. So we need
    /// an indipendent test to have an idea of how well we'll do outside the
    /// current arena.
    /// The user provides a dataset and (optionally) a test set. Training set
    /// and validation set are automatically created from the dataset
    /// (see environment::validation_ratio).
    std::list<example> dataset_[k_max_dataset + 1];

    /// Used to select a subset of the active dataset.
    data::iterator end_[k_max_dataset + 1];

    /// Used to choose the data we want to operate on (training / validation
    /// set).
    /// begin(), end() and size() methods operate on the selected set.
    dataset_t active_dataset_;
  };

  template<class T>
  T data::example::cast_output() const
  {
    switch (d_output)
    {
    case d_bool:    return static_cast<T>(any_cast<bool>(output));
    case d_int:     return static_cast<T>(any_cast<int>(output));
    case d_double:  return static_cast<T>(any_cast<double>(output));
    default:        return static_cast<T>(0.0);
    }
  }
}  // namespace vita

#endif  // DATA_H
