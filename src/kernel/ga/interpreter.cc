/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/interpreter.h"

namespace vita
{
///
/// \param[in] p an individual.
///
interpreter<i_ga>::interpreter(const i_ga *p) : core_interpreter(), p_(*p)
{
  assert(p);
}

///
/// \return an empty `any`.
///
any interpreter<i_ga>::run_nvi()
{
  return any();
}

///
/// \return the output value of the current terminal symbol.
///
double interpreter<i_ga>::fetch_param(unsigned i) const
{
  return p_[i];
}

///
/// \return the penalty for `this` individual.
///
/// Calls penalty_locus() using the default starting locus.
///
double interpreter<i_ga>::penalty_nvi()
{
  double n(0);

  for (const auto &l : p_)
    n += p_[l].sym->penalty(this);

  return n;
}

///
/// \param[in] i a pointer to an individual (used to check if the interpreter
///              is working on the expected individual).
/// \return `true` if the object passes the internal consistency check.
///
bool interpreter<i_ga>::debug_nvi(const void *i) const
{
  if (i && static_cast<const i_ga *>(i) != &p_)
    return false;

  return p_.debug();
}
}  // namespace vita
