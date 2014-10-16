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

#if !defined(VITA_LOCUS_H)
#define      VITA_LOCUS_H

#include <iostream>

#include "kernel/gene.h"

namespace vita
{
  static_assert(sizeof(index_t) <= sizeof(std::size_t),
                "index_t sizes expected to be <= std::size_t");
  static_assert(sizeof(category_t) <= sizeof(std::size_t),
                "category_t sizes expected to be <= std::size_t");

  struct locus
  {
    index_t       index;
    category_t category;

    locus operator+(index_t) const;
  };

  ///
  /// \param[in] l1 first locus.
  /// \param[in] l2 second locus.
  /// \return \c true if \a *this is equal to \a l2.
  ///
  inline bool operator==(const locus &l1, const locus &l2)
  {
    return l1.index == l2.index && l1.category == l2.category;
  }

  ///
  /// \param[in] l1 first locus.
  /// \param[in] l2 second locus.
  /// \return \c true if \a *this is not equal to \a l2.
  ///
  inline bool operator!=(const locus &l1, const locus &l2)
  {
    return l1.index != l2.index || l1.category != l2.category;
  }

  ///
  /// \param[in] l1 first locus.
  /// \param[in] l2 second locus.
  /// \return \c true if \a *this precedes \a l2 in lexicographic order
  ///         (http://en.wikipedia.org/wiki/Lexicographical_order).
  ///
  /// This operator is required by the STL std::map container.
  ///
  inline bool operator<(const locus &l1, const locus &l2)
  {
    return l1.index < l2.index ||
           (l1.index == l2.index && l1.category < l2.category);
  }

  ///
  /// \param[in] i displacement.
  /// \return a new locus obtained from \a this incrementing index component by
  ///         \a i (and not changing category component).
  ///
  inline locus locus::operator+(index_t i) const
  {
    return {index + i, category};
  }

  ///
  /// \param[out] s output stream.
  /// \param[in] l locus to print.
  /// \return output stream including \a l.
  ///
  inline std::ostream &operator<<(std::ostream &s, const locus &l)
  {
    return s << '(' << l.index << ',' << l.category << ')';
  }
}  // namespace vita

#endif  // Include guard