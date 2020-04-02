/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <iostream>

#include "kernel/value.h"

namespace vita
{

///
/// Streams a value_t object.
///
/// \param[out] o output stream
/// \param[in]  v value to be streamed
/// \return       a reference to the output stream
///
std::ostream &operator<<(std::ostream &o, const value_t &v)
{
  // A more general implementation at
  // https://stackoverflow.com/a/47169101/3235496

  std::visit([&](auto &&arg) { o << arg; }, v);
  return o;
}

}  // namespace vita
