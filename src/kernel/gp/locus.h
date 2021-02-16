/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LOCUS_H)
#define      VITA_LOCUS_H

#include <iostream>
#include <limits>

#include "kernel/common.h"

namespace vita
{
/// Index in the genome.
/// \note A locus is uniquely identified by an index **and** a category.
using index_t = std::size_t;

static_assert(sizeof(index_t) <= sizeof(std::size_t),
              "index_t size expected to be <= std::size_t");
static_assert(sizeof(category_t) <= sizeof(std::size_t),
              "category_t size expected to be <= std::size_t");

struct locus
{
  index_t       index;
  category_t category;

  static constexpr locus npos()
  {
    return {std::numeric_limits<index_t>::max(),
            std::numeric_limits<category_t>::max()};
  }
};

///
/// \param[in] l1 first locus
/// \param[in] l2 second locus
/// \return       `true` if `l1` is equal to `l2`
///
inline bool operator==(const locus &l1, const locus &l2)
{
  return l1.index == l2.index && l1.category == l2.category;
}

///
/// \param[in] l1 first locus
/// \param[in] l2 second locus
/// \return       `true` if `l1` is not equal to `l2`
///
inline bool operator!=(const locus &l1, const locus &l2)
{
  return l1.index != l2.index || l1.category != l2.category;
}

///
/// \param[in] l1 first locus
/// \param[in] l2 second locus
/// \return       `true` if `l1` precedes `l2` in lexicographic order
///               (http://en.wikipedia.org/wiki/Lexicographical_order)
///
/// This operator is required by the STL `std::map` container.
///
inline bool operator<(const locus &l1, const locus &l2)
{
  return l1.index < l2.index ||
         (l1.index == l2.index && l1.category < l2.category);
}

///
/// \param[in] l base locus
/// \param[in] i displacement
/// \return      a new locus obtained from `l` incrementing index component by
///              `i` (and not changing the category component)
///
inline locus operator+(const locus &l, index_t i)
{
  return {l.index + i, l.category};
}

///
/// \param[out] s output stream
/// \param[in]  l locus to print
/// \return       output stream including `l`
///
inline std::ostream &operator<<(std::ostream &s, const locus &l)
{
  return s << '[' << l.index << ',' << l.category << ']';
}
}  // namespace vita

#endif  // include guard
