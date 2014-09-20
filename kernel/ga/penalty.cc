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

#include "kernel/ga/penalty.h"
#include "kernel/ga/i_ga.h"
#include "kernel/ga/primitive.h"

namespace vita
{
  ///
  /// \param[in] ind an individual.
  /// \return the feasability degree of \a ind (0 means feasible; a positive
  ///         value means unfeasible. Greater values are worse).
  ///
  unsigned penalty(const i_ga &ind)
  {
    unsigned p(0);

    for (const auto &l : ind)
      p += static_cast<ga::real *>(ind[l].sym)->unfeasible(ind[l].par);

    return p;
  }
}  // namespace vita
