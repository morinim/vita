/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DATAFRAME_H)
#define      VITA_DATAFRAME_H

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "kernel/distribution.h"
#include "kernel/problem.h"
#include "utility/pocket_csv.h"

namespace vita
{
/// The type used as class ID in classification tasks.
using class_t = std::size_t;

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
/// - accepts many different kinds of input: CSV and XRFF files.
///
/// \see https://github.com/morinim/vita/wiki/dataframe
///
class dataframe
{
public:
  // ---- Structures ----
  struct example;
  class params;

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

  /// Information about the collection of columns (type, name, output index).
  class columns_info
  {
  public:
    /// Information about a single column of the dataset.
    struct column_info
    {
      std::string         name =     {};
      domain_t          domain = d_void;
      std::set<value_t> states =     {};
    };

    using size_type = std::size_t;

    columns_info();

    const column_info &operator[](size_type i) const { return cols_[i]; }
    column_info &operator[](size_type i) { return cols_[i]; }

    size_type size() const { return cols_.size(); }
    bool empty() const { return cols_.empty(); }

    auto begin() const { return cols_.begin(); }
    auto begin() { return cols_.begin(); }
    auto end() const { return cols_.end(); }
    auto end() { return cols_.end(); }

    const auto &front() const { return cols_.front(); }
    auto &front() { return cols_.front(); }

    const auto &back() const { return cols_.back(); }
    auto &back() { return cols_.back(); }

    void pop_back() { cols_.pop_back(); }
    void push_back(const column_info &);
    void push_front(const column_info &);

    void build(const record_t &, bool);

    bool is_valid() const;

  private:
    std::vector<column_info> cols_;
  };

  // ---- Constructors ----
  dataframe();
  explicit dataframe(std::istream &);
  dataframe(std::istream &, const params &);
  explicit dataframe(const std::filesystem::path &);
  dataframe(const std::filesystem::path &, const params &);

  // ---- Iterators ----
  using iterator = typename examples_t::iterator;
  using const_iterator = typename examples_t::const_iterator;
  using difference_type = typename examples_t::difference_type;

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  value_type front() const;
  value_type &front();

  // ---- Modifiers ----
  void clear();
  iterator erase(iterator, iterator);

  // ---- Convenience ----
  std::size_t read(const std::filesystem::path &);
  std::size_t read(const std::filesystem::path &, const params &);
  std::size_t read_csv(std::istream &);
  std::size_t read_csv(std::istream &, params);
  std::size_t read_xrff(std::istream &);
  std::size_t read_xrff(std::istream &, const params &);
  bool operator!() const;

  void push_back(const example &);

  std::size_t size() const;
  bool empty() const;

  class_t classes() const;
  unsigned variables() const;

  std::string class_name(class_t) const;

  bool is_valid() const;

  columns_info columns;

private:
  bool read_record(const record_t &, bool);
  example to_example(const record_t &, bool);

  class_t encode(const std::string &);

  std::size_t read_csv(const std::filesystem::path &, const params &);
  std::size_t read_xrff(const std::filesystem::path &, const params &);
  std::size_t read_xrff(tinyxml2::XMLDocument &, const params &);

  // Integer are simpler to manage than textual data, so, when appropriate,
  // input strings are converted into integers by this map and the `encode`
  // static function.
  std::map<std::string, class_t> classes_map_;

  // Available data.
  examples_t dataset_;
};

domain_t from_weka(const std::string &);

///
/// Stores a single element (row) of the dataset.
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
  /// The thing about which we want to make a prediction (aka instance). The
  /// elements of the vector are features.
  std::vector<value_t> input = {};
  /// The answer for the prediction task either the answer produced by the
  /// machine learning system, or the right answer supplied in the training
  /// data.
  value_t             output = {};

  std::uintmax_t difficulty  =  0;
  unsigned              age  =  0;

  void clear() { *this = example(); }
};

///
/// Get the output value for a given example.
///
/// \tparam    T the result is casted to type `T`
/// \param[in] e an example
/// \return      the output value for example  `e`
///
///
template<class T>
T label_as(const dataframe::example &e)
{
  return lexical_cast<T>(e.output);
}

///
/// Gets the `class_t` ID (aka label) for a given example.
///
/// \param[in] e an example
/// \return      the label of `e`
///
/// \warning Used only in classification tasks.
///
inline class_t label(const dataframe::example &e)
{
  Expects(std::holds_alternative<D_INT>(e.output));
  return std::get<D_INT>(e.output);
}

class dataframe::params
{
public:
  params &header()
  { dialect.has_header = pocket_csv::dialect::HAS_HEADER; return *this; }
  params &no_header()
  { dialect.has_header =  pocket_csv::dialect::NO_HEADER; return *this; }

  params &output(std::size_t o) { output_index =            o; return *this; }
  params &no_output()           { output_index = std::nullopt; return *this; }

  /// \remark Used only when reading CSV files.
  pocket_csv::dialect dialect = {};

  /// A filter and transform function applied when reading data.
  filter_hook_t filter = nullptr;

  /// Index of the column containing the output value (label).
  /// \remark Used only when reading CSV files.
  std::optional<std::size_t> output_index = 0;
};

}  // namespace vita

#endif  // include guard
