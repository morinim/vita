/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
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

#include "kernel/common.h"
#include "kernel/value.h"
#include "kernel/gp/src/dataframe.h"

namespace vita
{

///
/// Category/type management of the dataframe columns.
///
/// - `weak`: columns having the same domain can be freely mixed by the engine.
/// - `strong`: every column has its own type/category (Strongly Typed Genetic
/// Programming).
///
/// Even when specifying `weakly_typed` the engine won't mix all the columns.
/// Particularly a unique category will be assigned to:
/// - columns associated with distinct domains;
/// - columns with `d_string` domain.
///
enum class typing {weak, strong};

struct category_info
{
  static const category_info null;

  category_t category = undefined_category;
  domain_t     domain =             d_void;
  std::string    name =                 {};
};

std::ostream &operator<<(std::ostream &, const category_info &);
bool operator==(const category_info &, const category_info &);

///
/// Information about the set of categories used in a specific problem.
///
/// More *fine grained* data types are required for Strongly Typed Genetic
/// Programming: i.e. not just `double` but possibly multiple categories
/// (`category_t`) that are subset of a `d_double` `domain_t`. Categories avoid
/// mixing 3 Km/h with 4 Kg (in simple cases there is only one category for
/// each domain).
///
/// The src_problem class uses a category_set object for creating the symbol
/// set.
///
/// \see https://github.com/morinim/vita/wiki/bibliography#14
///
class category_set
{
public:
  class const_iterator;

  explicit category_set(const dataframe::columns_info &, typing = typing::weak);

  const category_info &category(category_t) const;
  const category_info &column(std::size_t) const;
  const category_info &column(const std::string &) const;

  std::set<category_t> used_categories() const;

  auto begin() const { return columns_.begin(); }
  auto end() const { return columns_.end(); }

  bool is_valid() const;

private:
  std::vector<category_info> columns_;
};  // class category_set

}  // namespace vita
#endif  // include guard
