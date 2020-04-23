/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CSV_PARSER_H)
#define      VITA_CSV_PARSER_H

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <functional>
#include <sstream>

#include "kernel/common.h"

namespace vita
{

struct csv_dialect
{
  /// A one-character string used to separate fields.
  char delimiter = ',';
  /// When `true` skips leading and trailing spaces adjacent to commas.
  bool trim_ws = false;
  /// When `true` assumes a header row is present.
  bool has_header = true;
};  // class csv_dialect

csv_dialect csv_sniffer(std::istream &);

///
/// Simple parser for CSV files.
///
/// \warning The class doesn't support multi-line fields.
///
class csv_parser
{
public:
  using record_t = std::vector<std::string>;
  using filter_hook_t = std::function<bool (record_t &)>;

  explicit csv_parser(std::istream &);
  csv_parser(std::istream &, const csv_dialect &);

  const csv_dialect &dialect() const;

  csv_parser &delimiter(char) &;
  csv_parser delimiter(char) &&;

  csv_parser &trim_ws(bool) &;
  csv_parser trim_ws(bool) &&;

  csv_parser &filter_hook(filter_hook_t) &;
  csv_parser filter_hook(filter_hook_t) &&;

  class const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

private:
  // DO NOT move this. `is` must be initialized before other data members.
  std::istream *is_;

  filter_hook_t filter_hook_;
  csv_dialect dialect_;
};  // class csv_parser

///
/// A forward iterator for CSV records.
///
class csv_parser::const_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = csv_parser::record_t;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  const_iterator(std::istream *is = nullptr,
                 csv_parser::filter_hook_t f = nullptr,
                 const csv_dialect &dialect = {})
    : ptr_(is), filter_hook_(f), dialect_(dialect), value_()
  {
    if (ptr_)
      get_input();
  }

  /// \return an iterator pointing to the next record of the CSV file
  const_iterator &operator++()
  {
    get_input();
    return *this;
  }

  /// \return reference to the current record of the CSV file
  const_reference operator*() const { return value_; }

  /// \return pointer to the current record of the CSV file
  const_pointer operator->() const { return &operator*(); }

  /// \param[in] lhs first term of comparison
  /// \param[in] rhs second term of comparison
  /// \return        `true` if iterators point to the same line
  friend bool operator==(const const_iterator &lhs, const const_iterator &rhs)
  {
    return lhs.ptr_ == rhs.ptr_ && lhs.value_ == rhs.value_ &&
           (!lhs.ptr_ || lhs.ptr_->tellg() == rhs.ptr_->tellg());
  }

  friend bool operator!=(const const_iterator &lhs, const const_iterator &rhs)
  {
    return !(lhs == rhs);
  }

private:
  value_type parse_line(const std::string &);
  void get_input();

  // Data members MUST BE INITIALIZED IN THE CORRECT ORDER:
  // * `value_` is initialized via the `get_input` member function;
  // * `get_input` works only if `_ptr` is already initialized.
  // So it's important that `value_` is the last value to be initialized.
  std::istream *ptr_;
  csv_parser::filter_hook_t filter_hook_;
  csv_dialect dialect_;
  value_type value_;
};  // class csv_parser::const_iterator

}  // namespace vita

#endif  // include guard
