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

#if !defined(VITA_INDIVIDUAL_INL_H)
#define      VITA_INDIVIDUAL_INL_H

inline individual::individual(const environment &e, const symbol_set &ss)
  : signature_(), age_(0), env_(&e), sset_(&ss)
{
  assert(e.debug(true, true));

}

#endif  // Include guard
