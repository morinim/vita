/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CATEGORY_SET_H)
#define      VITA_CATEGORY_SET_H

#include <iostream>
#include <set>
#include <string>

#include "kernel/vita.h"

namespace vita
{
///
/// \brief Informations about a category of the dataset
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
/// is mapped to:
/// * {"", d_string, {"Iris-setosa", "Iris-versicolor", "Iris-virginica"}}
///
/// while
///
///     <attribute type="numeric" category="A" name="Speed" />
///
/// is mapped to:
/// * {"A", d_double, {}}
///
struct untagged_category
{
  std::string             name;
  domain_t              domain;
  std::set<std::string> labels;
};

///
/// \brief Integrates untagged_category struct with a unique identifier
///
struct category : public untagged_category
{
  category(category_t t, const untagged_category &uc)
    : untagged_category(uc), tag(t)
  {
  }

  explicit operator bool() const;

  category_t tag;

  /// A special value used for missing answers.
  static const category null;
};  // struct category

std::ostream &operator<<(std::ostream &, const category &);

///
/// \brief The set of categories used in a specific problem
///
class category_set
{
public:
  class const_iterator;

  category_set();

  const_iterator begin() const;
  const_iterator end() const;
  unsigned size() const;

  category find(const std::string &) const;
  category find(category_t) const;

  category_t insert(const untagged_category &);

  void add_label(category_t, const std::string &);
  void swap(category_t, category_t);

private:
  std::vector<untagged_category> categories_;

  // Initially we had a std::map from category name to category tag used
  // to speed up the find(std::string) method.
  // Now the implementation is a bit slower but simpler and more robust.
};  // class category_set

///
/// \brief Forward iterator to read the elements of a category_set
///
class category_set::const_iterator
{
private:  // Private data members
  using const_iter = std::vector<untagged_category>::const_iterator;

  const_iter begin_;
  const_iter ptr_;

public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = category;
  using pointer = value_type *;
  using reference = value_type &;

  const_iterator(const_iter b, const_iter c) : begin_(b), ptr_(c) {}

  const_iterator operator++() { ++ptr_; return *this; }
  value_type operator*()
  { return category(static_cast<category_t>(ptr_ - begin_), *ptr_); }

  bool operator==(const const_iterator &rhs) const
  { return ptr_ == rhs.ptr_; }
  bool operator!=(const const_iterator &rhs) const
  { return ptr_ != rhs.ptr_; }
};  // class category_set::const_iterator

}  // namespace vita
#endif  // Include guard
