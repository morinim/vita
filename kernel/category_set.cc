/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <iterator>

#include "kernel/category_set.h"

namespace vita
{
  const category category::null(0, {"", domain_t::d_void, {}});

  ///
  /// \return \c true if *this != category::null.
  ///
  category::operator bool() const
  {
    return tag != null.tag || name != null.name || domain != null.domain ||
           labels != null.labels;
  }

  ///
  /// \brief Builds an empty category_set
  ///
  category_set::category_set() : categories_()
  {
  }

  ///
  /// \brief Standard, STL-style, begin method
  ///
  category_set::const_iterator category_set::begin() const
  {
    return category_set::const_iterator(categories_.begin(),
                                        categories_.begin());
  }

  ///
  /// \brief Standard, STL-style, end method
  ///
  category_set::const_iterator category_set::end() const
  {
    return category_set::const_iterator(categories_.begin(),
                                        categories_.end());
  }

  ///
  /// \return number of categories.
  ///
  /// \attention
  /// please note that the value categories() returns may differ from the
  /// intuitive number of categories of the dataset (it can be 1 unit smaller).
  /// For instance consider the simple Iris classification problem:
  ///
  ///     ...
  ///     <attribute class="yes" name="class" type="nominal">
  ///       <labels>
  ///         <label>Iris-setosa</label> ... <label>Iris-virginica</label>
  ///       </labels>
  ///     </attribute>
  ///     <attribute name="sepallength" type="numeric" />
  ///     ...
  /// It has a nominal attribute to describe output classes and four numeric
  /// attributes as inputs. So there are two distinct attribute types
  /// (nominal and numeric), i.e. two categories.
  /// But... categories() would return 1.
  /// This happens because the genetic programming algorithm for classification
  /// we use (based on a discriminant function) doesn't manipulate (skips) the
  /// output category (it only uses the number of output classes).
  ///
  size_t category_set::size() const
  {
    return categories_.size();
  }

  ///
  /// \param[in] name name of a category.
  /// \return the category with the specified \a name (if it doesn't exist
  ///         returns category::null).
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

    const auto tag(std::distance(categories_.begin(), uc));
    return category(tag, categories_[tag]);
  }

  ///
  /// \param[in] t tag of a category.
  /// \return a the category with the spcified \a t (if it doesn't exist
  ///         returns category::null).
  ///
  category category_set::find(category_t t) const
  {
    return t < size() ? category(t, categories_[t]) : category::null;
  }

  ///
  /// \param[in] a new untagged_category for the set.
  /// \return the tag associated with \a c.
  ///
  category_t category_set::insert(const untagged_category &c)
  {
    assert(!c.name.empty());

    category c1(find(c.name));

    if (!c1)
    {
      c1.tag = size();
      categories_.push_back(c);
    }

    return c1.tag;
  }

  ///
  /// \param[in] t the tag of a category.
  /// \param[in] l a label.
  ///
  /// Add the label \a l to the set of labels associated with category \a t.
  ///
  void category_set::add_label(category_t t, const std::string &l)
  {
    assert(t < size());
    assert(categories_[t].domain == domain_t::d_string);

    categories_[t].labels.insert(l);
  }

  ///
  /// \param[in] t1 a tag of a category.
  /// \param[in] t2 a tag of category.
  ///
  /// Swap categories \a t1 and \a t2.
  ///
  void category_set::swap(category_t t1, category_t t2)
  {
    assert(t1 < size());
    assert(t2 < size());

    std::swap(categories_[t1], categories_[t2]);
  }

  ///
  /// \param[out] s output stream.
  /// \param[in] c category to print.
  /// \return output stream including \a c.
  ///
  /// Utility function used for debugging purpose.
  ///
  std::ostream &operator<<(std::ostream &s, const category &c)
  {
    s << c.name << " (category " << c.tag << ", domain "
      << static_cast<std::underlying_type<domain_t>::type>(c.domain);

    s << ", [";

    if (!c.labels.empty())
      std::copy(c.labels.begin(), c.labels.end(),
                std::ostream_iterator<std::string>(s, " "));

    s << "])";

    return s;
  }
}  // namespace vita
