/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/individual.h"

namespace vita
{
///
/// \param[in] env environment used for building the individual.
/// \param[in] in input stream.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the object isn't modified.
///
bool individual::load(std::istream &in, const environment &env)
{
  decltype(age()) t_age;
  if (!(in >> t_age))
    return false;

  if (!load_nvi(in, env))
    return false;

  age_ = t_age;

  // We don't save/load signature: it can be easily calculated on the fly.
  signature_.clear();

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if the object has been saved correctly.
///
bool individual::save(std::ostream &out) const
{
  out << age() << '\n';

  return save_nvi(out);
}

///
/// \param[in] rhs_age the age of an individual.
///
/// Updates the age of this individual if it's smaller than `rhs_age`.
///
void individual::set_older_age(unsigned rhs_age)
{
  if (age() < rhs_age)
    age_ = rhs_age;
}

}  // namespace vita
