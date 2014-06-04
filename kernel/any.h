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

#if defined(USE_SPIRIT)
#  include <boost/spirit/home/support/detail/hold_any.hpp>
#else
#  include <boost/any.hpp>
#endif

#include "kernel/vita.h"

namespace vita
{
  ///
  /// A shortcut for the any type (usually boost::any or boost::spirit::any).
  ///
#if defined(USE_SPIRIT)
  using any = boost::spirit::hold_any;
#else
  using any = boost::any;
#endif

  ///
  /// \param a an any.
  /// \return the value contained in \a a.
  ///
  template<class T> inline T anycast(const any &a)
  {
#if defined(USE_SPIRIT)
    return boost::spirit::any_cast<T>(a);
#else
    return boost::any_cast<T>(a);
#endif
  }

  ///
  /// \param a pointer to any.
  /// \return a pointer to the value contained in \a a.
  ///
  template<class T> inline const T *anycast(const any *a)
  {
#if defined(USE_SPIRIT)
    return boost::spirit::any_cast<T>(a);
#else
    return boost::any_cast<T>(a);
#endif
  }

  template<class T> T to(const any &);
}  // namespace vita

#endif  // Include guard
