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

#if !defined(VITA_COMP_PENALTY_H)
#define      VITA_COMP_PENALTY_H

#include "kernel/interpreter.h"

namespace vita
{
  ///
  /// \return 0 / 1 / 2
  ///
  /// A convenient function to "factorize" the penalty function of a typical
  /// comparison class.
  ///
  /// Maximum penalty for
  ///     if ("apple" == "apple") then "orange" else "orange"
  ///
  unsigned comparison_function_penalty(core_interpreter *ci)
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));

    const auto id0(i->fetch_index(0));
    const auto id1(i->fetch_index(1));
    const auto id2(i->fetch_index(2));
    const auto id3(i->fetch_index(3));

    return (id0 == id1) + (id2 == id3);
  };
}  // namespace vita

#endif  // Include guard
