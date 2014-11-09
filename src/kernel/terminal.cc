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

#if !defined(VITA_NO_LIB)
#  include "kernel/terminal.h"
#endif

namespace vita
{
  ///
  /// \return \c true if the \a function passes the internal consistency check.
  ///
  VITA_INLINE
  bool terminal::debug() const
  {
    if (arity_)  // This is a terminal, so there shouldn't be arguments
      return false;

    if (associative_)
      return false;

    return symbol::debug();
  }
}  // namespace vita
