/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <iterator>

#include "kernel/category_set.h"
#include "utility/utility.h"

namespace vita
{
const category category::null(0, {"", domain_t::d_void, {}});

///
/// \return `true` if `*this != category::null`
///
category::operator bool() const
{
  return tag != null.tag || name != null.name || domain != null.domain
         || labels != null.labels;
}

///
/// Builds an empty category_set.
///
category_set::category_set() : categories_()
{
}

///
/// Standard, STL-style, begin method.
///
category_set::const_iterator category_set::begin() const
{
  return category_set::const_iterator(categories_.begin(),
                                      categories_.begin());
}

///
/// Standard, STL-style, end method.
///
category_set::const_iterator category_set::end() const
{
  return category_set::const_iterator(categories_.begin(),
                                      categories_.end());
}

///
/// Number of input categories.
///
/// \return number of input categories
///
/// Please note that the value returned may differ from the intuitive number of
/// categories of the dataset (it could be `1` unit smaller).
/// For instance consider the simple *Iris* classification problem:
///
///     ...
///     <attribute class="yes" name="class" type="nominal">
///       <labels>
///         <label>Iris-setosa</label> ... <label>Iris-virginica</label>
///       </labels>
///     </attribute>
///     <attribute name="sepallength" type="numeric" />
///     ...
///
/// There is a nominal attribute to describe output classes and four numeric
/// attributes as input. So two distinct attributes / categories (nominal and
/// numeric) but... `size()` returns `1` (the number of input categories).
///
/// This choice is due to the fact that:
/// - for regression problems the output category is also in the set of input
///   categories;
/// - for classification tasks the algorithm ignores the output category
///   (because it's based on a discriminant function).
///
category_t category_set::size() const
{
  return static_cast<category_t>(categories_.size());
}

///
/// \param[in] name name of a category
/// \return    the category with the specified `name` or `category::null` if it
///            doesn't exists
///
category category_set::find(const std::string &name) const
{
  const auto uc(std::find_if(categories_.begin(), categories_.end(),
                             [name](const untagged_category &c)
                             {
                               return c.name == name;
                             }));

  if (uc == categories_.end())
    return category::null;

  const auto tag(
    static_cast<category_t>(std::distance(categories_.begin(), uc)));
  return category(tag, categories_[tag]);
}

///
/// \param[in] t tag of a category
/// \return      the category with tag `t` or `category::null` if it doesn't
///              exists)
///
category category_set::find(category_t t) const
{
  return t < size() ? category(t, categories_[t]) : category::null;
}

///
/// \param[in] c a new untagged_category for the set
/// \return      the tag associated with `c`
///
category_t category_set::insert(const untagged_category &c)
{
  Expects(!c.name.empty());

  category c1(find(c.name));

  if (!c1)
  {
    c1.tag = static_cast<decltype(c1.tag)>(size());
    categories_.push_back(c);
  }

  return c1.tag;
}

///
/// Adds a label to the set of labels associated with a category.
///
/// \param[in] t the tag of the category `l` should be added to
/// \param[in] l label to be added
///
void category_set::add_label(category_t t, const std::string &l)
{
  Expects(t < size());
  Expects(categories_[t].domain == domain_t::d_string);

  categories_[t].labels.insert(l);
}

///
/// Swaps two categories.
///
/// \param[in] t1 a tag of a category
/// \param[in] t2 a tag of a category
///
void category_set::swap(category_t t1, category_t t2)
{
  Expects(t1 < size());
  Expects(t2 < size());

  std::swap(categories_[t1], categories_[t2]);
}

///
/// Utility function used for debugging purpose.
///
/// \param[out] s output stream
/// \param[in] c  category to print
/// \return       output stream including `c`
///
std::ostream &operator<<(std::ostream &s, const category &c)
{
  s << c.name << " (category " << c.tag << ", domain " << c.domain;

  s << ", [";

  if (!c.labels.empty())
    std::copy(c.labels.begin(), c.labels.end(),
              infix_iterator<std::string>(s, " "));

  s << "])";

  return s;
}

}  // namespace vita
