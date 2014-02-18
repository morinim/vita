/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ANY_H)
#define      VITA_ANY_H

#include <boost/spirit/home/support/detail/hold_any.hpp>

#include "kernel/vita.h"

namespace vita
{
  ///
  /// A shortcut for the any type (usually boost::any or boost::spirit::any).
  ///
  typedef boost::spirit::hold_any any;

  ///
  /// \param a an any.
  /// \return the value contained in \a a.
  ///
  template<class T> inline T any_cast(const any &a)
  {
    // We must choose the right any_cast (it depends on any typedef).
    // The alternative is: return boost::any_cast<T>(a);
    return boost::spirit::any_cast<T>(a);
  }

  template<class T> T to(const any &);
}  // namespace vita

#endif  // Include guard
