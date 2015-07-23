/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
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
/// \return `true` if the function passes the internal consistency check.
///
VITA_INLINE
bool terminal::debug() const
{
  if (associative())
    return false;

  return symbol::debug();
}
}  // namespace vita
