/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_DATA_H)
#define      VITA_SRC_DATA_H

#include <map>
#include <string>

#include "kernel/category_set.h"
#include "kernel/data.h"
#include "kernel/distribution.h"
#include "utility/any.h"

namespace vita
{
/// \brief The type used as class id in classification tasks.
using class_t = unsigned;

///
/// \brief Implementation of vita::data for symbolic regression and
///        classification tasks.
///
/// src_data is a collection of "monomorphic" examples (all samples have the
/// same type and arity) that is forward iterable.
///
/// It can read xrff (http://weka.wikispaces.com/XRFF) and CSV
/// (https://developers.google.com/prediction/docs/developer-guide?hl=it)
/// files.
///
class src_data : public data
{
public:
  // ---- Structures ----
  struct example;
  struct column;

  // ---- Aliases ----
  using examples_t = std::vector<example>;

  /// Raw input record. The ETL chain is:
  /// > FILE -> record_t -> example --(vita::push_back)--> vita::src_data
  using record_t = std::vector<std::string>;

  /// A filter and transform function (returns `true` for records that should
  /// be loaded and, possibly, transform its input parameter).
  using filter_hook_t = std::function<bool (record_t *)>;

  // ---- Constructors ----
  src_data();
  explicit src_data(const std::string &, filter_hook_t = nullptr);

  // ---- Iterators ----
  using iterator = typename examples_t::iterator;
  using const_iterator = typename examples_t::const_iterator;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  // ---- Convenience ----
  std::size_t load(const std::string &, filter_hook_t = nullptr);
  bool operator!() const;

  void push_back(const example &);

  void slice(std::size_t);

  void clear();

  void partition(unsigned);
  void sort(std::function<bool (const example &, const example &)>);

  const category_set &categories() const;

  const column &get_column(unsigned) const;

  std::size_t size() const;
  std::size_t size(dataset_t) const;

  virtual bool has(dataset_t) const override;

  unsigned classes() const;
  unsigned columns() const;
  unsigned variables() const;

  std::string class_name(class_t) const;

  bool debug() const;

  static domain_t from_weka(const std::string &);

private: // Private support methods
  example to_example(const std::vector<std::string> &, bool, bool);

  class_t encode(const std::string &);

  std::size_t load_csv(const std::string &, filter_hook_t);
  std::size_t load_xrff(const std::string &, filter_hook_t);

  void swap_category(category_t, category_t);

private:  // Private data members
  // Integer are simpler to manage than textual data, so, when appropriate,
  // input strings are converted into integers by these maps (and the `encode`
  // static function).
  std::map<std::string, class_t> classes_map_;

  // How is the dataset organized? Sometimes we have a dataset header (XRFF
  // file format), other times it has to be implicitly derived (e.g. CSV).
  std::vector<column> header_;

  // What are the categories we are dealing with?
  category_set categories_;

  // Data are stored in three datasets:
  // * a training set used directly for learning;
  // * a validation set for controlling overfitting and measuring the
  //   performance of an individual;
  // * a test set for a forecast of how well an individual will do in the
  //   real world.
  // We don't validate on the training data because that would overfit the
  // model. We don't stop at the validation step because we've iteratively
  // been adjusting things to get a winner in the validation step. So we need
  // an independent test to have an idea of how well we'll do outside the
  // current arena.
  // The user provides a dataset and (optionally) a test set. Training set
  // and validation set are automatically created from the dataset
  // (see environment::validation_ratio).
  std::vector<examples_t> datasets_;

  // Used to keep track of subset of the dataset.
  std::vector<std::size_t> slice_;
};

///
/// \brief Stores a single element of the data set
///
/// The `struct` consists of an input vector (`input`) and an answer value
/// (`output`). Depending on the kind of problem, `output` stores:
/// * a numeric value (symbolic regression problem);
/// * a label (classification problem).
///
/// `difficulty` and `age` are parameters used by the Dynamic Subset
/// Selection algorithm (see "Dynamic Training Subset Selection for
/// Supervised Learning in Genetic Programming" - Chris Gathercole, Peter
/// Ross).
///
struct src_data::example
{
  example() : input(), output(any()), difficulty(0), age(0) {}

  std::vector<any> input;
  any             output;

  std::uintmax_t difficulty;
  unsigned              age;

  class_t tag() const { return any_cast<class_t>(output); }
  template<class T> T cast_output() const;

  void clear() { *this = example(); }
};

template<class T>
T src_data::example::cast_output() const
{
  if (auto *v = any_cast<double>(&output))
    return static_cast<T>(*v);

  if (auto *v = any_cast<int>(&output))
    return static_cast<T>(*v);

  if (auto *v = any_cast<bool>(&output))
    return static_cast<T>(*v);

  return static_cast<T>(0.0);
}

///
/// \brief Informations about a "column" (feature) of the dataset
///
struct src_data::column
{
  std::string       name;
  category_t category_id;
};

}  // namespace vita

#endif  // include guard
