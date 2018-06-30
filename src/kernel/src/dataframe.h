/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DATAFRAME_H)
#define      VITA_DATAFRAME_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "kernel/distribution.h"
#include "kernel/problem.h"
#include "kernel/src/category_set.h"
#include "utility/any.h"

namespace vita
{
/// The type used as class id in classification tasks.
using class_t = unsigned;

///
/// A 2-dimensional labeled data structure with columns of potentially
/// different types.
///
/// You can think of it like a spreadsheet or SQL table.
///
/// Dataframe:
/// - is modelled on the corresponding *pandas* object;
/// - is a forward iterable collection of "monomorphic" examples (all samples
///   have the same type and arity);
/// - accepts many different kinds of input: XRFF
///   (http://weka.wikispaces.com/XRFF) and CSV files.
///
class dataframe
{
public:
  // ---- Structures ----
  struct example;
  struct column;

  // ---- Aliases ----
  using examples_t = std::vector<example>;
  using value_type = typename examples_t::value_type;

  /// Raw input record.
  /// The ETL chain is:
  /// > FILE -> record_t -> example --(vita::push_back)--> vita::dataframe
  using record_t = std::vector<std::string>;

  /// A filter and transform function (returns `true` for records that should
  /// be loaded and, possibly, transform its input parameter).
  using filter_hook_t = std::function<bool (record_t &)>;

  // ---- Constructors ----
  dataframe();
  explicit dataframe(const std::string &, filter_hook_t = nullptr);

  // ---- Iterators ----
  using iterator = typename examples_t::iterator;
  using const_iterator = typename examples_t::const_iterator;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  // ---- Modifiers ----
  void clear();
  iterator erase(iterator, iterator);

  // ---- Convenience ----
  std::size_t load(const std::string &, filter_hook_t = nullptr);
  bool operator!() const;

  void push_back(const example &);

  const category_set &categories() const;

  const column &get_column(unsigned) const;

  std::size_t size() const;
  bool empty() const;

  unsigned classes() const;
  unsigned columns() const;
  unsigned variables() const;

  std::string class_name(class_t) const;

  bool debug() const;

  static domain_t from_weka(const std::string &);

private:
  example to_example(const std::vector<std::string> &, bool, bool);

  class_t encode(const std::string &);

  std::size_t load_csv(const std::string &, filter_hook_t);
  std::size_t load_xrff(const std::string &, filter_hook_t);

  void swap_category(category_t, category_t);

  // Integer are simpler to manage than textual data, so, when appropriate,
  // input strings are converted into integers by these maps (and the `encode`
  // static function).
  std::map<std::string, class_t> classes_map_;

  // How is the dataset organized? Sometimes we have a dataset header (XRFF
  // file format), other times it has to be implicitly derived (e.g. CSV).
  std::vector<column> header_;

  // What are the categories we are dealing with?
  category_set categories_;

  // Available data.
  examples_t dataset_;
};

///
/// Stores a single element of the dataset.
///
/// The `struct` consists of an input vector (`input`) and an answer value
/// (`output`). Depending on the kind of problem, `output` stores:
/// * a numeric value (symbolic regression problem);
/// * a categorical value (classification problem).
///
/// `difficulty` and `age` are parameters used by the Dynamic Subset
/// Selection algorithm (see "Dynamic Training Subset Selection for
/// Supervised Learning in Genetic Programming" - Chris Gathercole, Peter
/// Ross).
///
struct dataframe::example
{
  std::vector<any> input = {};
  any             output = {};

  std::uintmax_t difficulty = 0;
  unsigned              age = 0;

  class_t tag() const { return any_cast<class_t>(output); }
  template<class T> T cast_output() const;

  void clear() { *this = example(); }
};

template<class T>
T dataframe::example::cast_output() const
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
/// Informations about a "column" (feature) of the dataset.
///
struct dataframe::column
{
  std::string       name;
  category_t category_id;
};

}  // namespace vita

#endif  // include guard
