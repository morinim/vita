/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_TRILEAN_H)
#define      VITA_TRILEAN_H

#include <iostream>

#include "kernel/common.h"

namespace vita
{

///
/// Three-valued logic enum.
///
/// Used where we need three truth values indicating `true`, `false` and some
/// indeterminate third value.
///
/// \note
/// Boost::tribool already implements a similar object and in the past we were
/// using it. Logical operators in `boost:tribool` are overloaded and unlike
/// built-in logical operators of C++ the left to right evaluation is not
/// applied. There is no short circuiting so the order-evaluation of operands
/// remains unspecified. As a result we decided to switch to something with
/// less syntactic sugar but with a less unexpected behaviour.
///
enum class trilean {unknown = -1, no, yes};

inline std::istream &operator>>(std::iostream &i, trilean &v)
{
  int tmp;

  if (i >> tmp)
    switch (tmp)
    {
    case 0:  v =      trilean::no; break;
    case 1:  v =     trilean::yes; break;
    default: v = trilean::unknown;
    }

  return i;
}

inline trilean &assign(trilean &lhs, bool rhs)
{
  return lhs = rhs ? trilean::yes : trilean::no;
}

}  // namespace vita

#endif  // include guard
