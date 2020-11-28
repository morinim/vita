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

#include <algorithm>
#include <iterator>

#include "kernel/gp/src/category_set.h"
#include "utility/utility.h"

namespace vita
{

const category_info category_info::null = {};

///
/// Builds a category_set extracting data from a columns_info object.
///
/// \param[in] cols columns of a dataframe
/// \param[in] t    `weak` or `strong` (see `typing`)
///
category_set::category_set(const dataframe::columns_info &cols, typing t)
  : columns_()
{
  category_t categories(0);

  for (const auto &c : cols)
  {
    category_t id;
    if (c.domain == d_void)
      id = undefined_category;
    else if (t == typing::strong || c.domain == d_string)
      id = categories++;
    else
    {
      const auto it(std::find_if(begin(), end(),
                                 [target = c.domain](const auto &x)
                                 {
                                   return x.domain == target;
                                 }));

      if (it == columns_.end())
        id = categories++;
      else
        id = it->category;
    }

    columns_.push_back({id, c.domain, c.name});
  }
}

///
/// \param[in] category a category
/// \return             information about column 'category'
///
const category_info &category_set::category(category_t category) const
{
  const auto it(std::find_if(begin(), end(),
                             [category](const auto &e)
                             {
                               return e.category == category;
                             }));
  return it == columns_.end() ? category_info::null : *it;
}

///
/// \param[in] i index of a dataframe column
/// \return    information about column 'i'
///
const category_info &category_set::column(std::size_t i) const
{
  Expects(i < columns_.size());
  return columns_[i];
}

///
/// \param[in] name column name
/// \return         information about column 'name'
///
const category_info &category_set::column(const std::string &name) const
{
  const auto it(std::find_if(begin(), end(),
                             [name](const auto &e)
                             {
                               return e.name == name;
                             }));
  return it == columns_.end() ? category_info::null : *it;
}

///
/// \return the set of used categories
///
std::set<category_t> category_set::used_categories() const
{
  std::set<category_t> ret;

  std::transform(begin(), end(), std::inserter(ret, ret.end()),
                 [](const auto &c) { return c.category; });

  return ret;
}

///
/// \return `true` if the object satisfies class invariants
///
bool category_set::is_valid() const
{
  // Unique column name (when available).
  for (std::size_t i(0); i < columns_.size(); ++i)
    if (!columns_[i].name.empty())
      for (std::size_t j(i + 1); j < columns_.size(); ++j)
        if (columns_[j].name == columns_[i].name)
          return false;

  // Undefined category implies void domain.
  for (std::size_t i(0); i < columns_.size(); ++i)
    if (columns_[i].category == undefined_category
        && columns_[i].domain != d_void)
      return false;

  // Same category implies same domain.
  for (std::size_t i(0); i < columns_.size(); ++i)
  {
    const category_t category(columns_[i].category);
    const domain_t domain(columns_[i].domain);

    for (std::size_t j(i + 1); j < columns_.size(); ++j)
      if (columns_[j].category == category && columns_[j].domain != domain)
        return false;
  }

  return true;
}

///
/// Utility function used for debugging purpose.
///
/// \param[out] s output stream
/// \param[in] c  category to print
/// \return       output stream including `c`
///
std::ostream &operator<<(std::ostream &s, const category_info &c)
{
  return s << c.name << " (category " << c.category << ", domain " << c.domain;
}

///
/// Compares two category_info structs for equality.
///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparison
/// return         `true` when `lhs` and `rhs` compare equals
///
bool operator==(const category_info &lhs, const category_info &rhs)
{
  return lhs.category == rhs.category && lhs.domain == rhs.domain
         && lhs.name == rhs.name;
}

}  // namespace vita
