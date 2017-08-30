/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <iostream>

#include "kernel/cache_hash.h"

namespace vita
{
///
/// \param[in] in input stream
/// \return       `true` if hash_t loaded correctly
///
/// \note
/// If the load operation isn't successful the current hash_t isn't changed.
///
bool hash_t::load(std::istream &in)
{
  hash_t tmp;

  if (!(in >> tmp.data[0] >> tmp.data[1]))
    return false;

  *this = tmp;

  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if hash_t was saved correctly
///
bool hash_t::save(std::ostream &out) const
{
  out << data[0] << ' ' << data[1] << '\n';

  return out.good();
}

///
/// Mainly useful for debugging / testing.
///
/// \param[out] o output stream
/// \param[in]  h hash signature to be printed
/// \return       the updated ostream
///
std::ostream &operator<<(std::ostream &o, hash_t h)
{
  return o << h.data[0] << h.data[1];
}

}  // namespace vita
