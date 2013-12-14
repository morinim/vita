/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/analyzer.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \tparam T type of individual.
  ///
  /// \param[in] ind individual to be analyzed.
  /// \return effective length of individual we gathered statistics about.
  ///
  template<>
  unsigned analyzer<individual>::count(const individual &ind)
  {
    for (index_t i(0); i < ind.size(); ++i)
      for (category_t c(0); c < ind.sset().categories(); ++c)
        count(ind[{i, c}].sym, false);

    unsigned length(0);
    for (const auto &l : ind)
    {
      count(ind[l].sym, true);
      ++length;
    }

    return length;
  }
}  // namespace vita
