/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DATA_H)
#define      VITA_DATA_H

#include <map>
#include <string>
#include <vector>

#include "kernel/any.h"
#include "kernel/category_set.h"
#include "kernel/distribution.h"

namespace vita
{
  ///
  /// \brief The type used as class id in classification tasks
  ///
  using class_t = unsigned;

  ///
  /// \brief Stores the dataset used to evolve vita::population
  ///
  /// It can read xrff (http://weka.wikispaces.com/XRFF) and CSV
  /// (https://developers.google.com/prediction/docs/developer-guide?hl=it)
  /// files.
  ///
  class data
  {
  public:  // Structures and type alias
    struct example;
    struct column;

    /// example *
    using iterator = typename std::vector<example>::iterator;
    /// const example *
    using const_iterator = typename std::vector<example>::const_iterator;

    enum dataset_t {training = 0, validation, test, k_sup_dataset};

  public:  // Construction, convenience
    data();
    explicit data(const std::string &, unsigned = 0);

    void dataset(dataset_t);
    dataset_t dataset() const;
    void slice(std::size_t);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    std::size_t size() const;
    std::size_t size(dataset_t) const;

    std::size_t open(const std::string &, unsigned = 0);
    bool operator!() const;

    void clear();

    void divide(unsigned);
    void sort(std::function<bool (const example &, const example &)>);

    const category_set &categories() const;

    const column &get_column(unsigned) const;

    unsigned classes() const;
    unsigned columns() const;
    unsigned variables() const;

    std::string class_name(class_t) const;

    bool debug() const;

    static domain_t from_weka(const std::string &);

  private: // Private support methods
    class_t encode(const std::string &);
    static std::vector<std::string> csvline(const std::string &, char = ',',
                                            bool = false);

    std::size_t load_csv(const std::string &, unsigned);
    std::size_t load_xrff(const std::string &);

    void swap_category(category_t, category_t);

  private:  // Private data members
    /// Integer are simpler to manage than textual data, so, when appropriate,
    /// input strings are converted into integers by these maps (and the encode
    /// static function).
    std::map<std::string, class_t> classes_map_;

    /// How is the dataset organized? Sometimes we have a dataset header (XRFF
    /// file format), other times it has to be implicitly derived (e.g. CSV).
    std::vector<column> header_;

    /// What are the categories we are dealing with?
    category_set categories_;

    /// Data are stored in three datasets:
    /// * a training set used directly for learning;
    /// * a validation set for controlling overfitting and measuring the
    ///   performance of an individual;
    /// * a test set for a forecast of how well an individual will do in the
    ///   real world.
    /// We don't validate on the training data because that would overfit the
    /// model. We don't stop at the validation step because we've iteratively
    /// been adjusting things to get a winner in the validation step. So we need
    /// an independent test to have an idea of how well we'll do outside the
    /// current arena.
    /// The user provides a dataset and (optionally) a test set. Training set
    /// and validation set are automatically created from the dataset
    /// (see environment::validation_ratio).
    std::vector<std::vector<example>> dataset_;

    /// Used to keep track of subset of the dataset.
    std::vector<std::size_t> slice_;

    /// Used to choose the data we want to operate on (training / validation
    /// set).
    /// begin(), end() and size() methods operate on the selected set.
    dataset_t active_dataset_;
  };

  ///
  /// \brief Stores a single element of the data set.
  ///
  /// The \c struct consists of an input vector (\a input) and an answer value
  /// (\a output). Depending on the kind of problem, \a output stores:
  /// * a numeric value (symbolic regression problem);
  /// * a label (classification problem).
  ///
  /// \a difficulty and \a age are parameters used by the Dynamic Subset
  /// Selection algorithm (see "Dynamic Training Subset Selection for
  /// Supervised Learning in Genetic Programming" - Chris Gathercole, Peter
  /// Ross).
  ///
  struct data::example
  {
    example() : input(), output(any()), d_output(domain_t::d_void),
                difficulty(0), age(0) {}

    std::vector<any> input;
    any             output;
    domain_t      d_output;

    std::uintmax_t  difficulty;
    unsigned               age;

    class_t tag() const { return anycast<class_t>(output); }
    template<class T> T cast_output() const;

    void clear() { *this = example(); }
  };

  template<class T>
  T data::example::cast_output() const
  {
    switch (d_output)
    {
    case domain_t::d_bool:    return static_cast<T>(anycast<bool>(output));
    case domain_t::d_int:     return static_cast<T>(anycast<int>(output));
    case domain_t::d_double:  return static_cast<T>(anycast<double>(output));
    default:                  return static_cast<T>(0.0);
    }
  }

  ///
  /// \brief Informations about a "column" (feature) of the dataset
  ///
  struct data::column
  {
    std::string       name;
    category_t category_id;
  };
}  // namespace vita

#endif  // Include guard
