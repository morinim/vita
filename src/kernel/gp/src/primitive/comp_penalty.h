/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_COMP_PENALTY_H)
#define      VITA_COMP_PENALTY_H

#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/mep/interpreter.h"

namespace vita
{
///
/// A simple, convenient function for the penalty score of the typical
/// four-terms comparison.
///
/// \return a penalty in the `{0, 1, 2}` set
///
/// Maximum penalty for `if ("apple" == "apple") then "orange" else "orange"`
///
[[nodiscard]] inline double comparison_function_penalty(core_interpreter *ci)
{
  auto i(static_cast<interpreter<i_mep> *>(ci));

  const auto id0(i->fetch_index(0));
  const auto id1(i->fetch_index(1));
  const auto id2(i->fetch_index(2));
  const auto id3(i->fetch_index(3));

  return (id0 == id1) + (id2 == id3);
}
}  // namespace vita

#endif  // include guard
