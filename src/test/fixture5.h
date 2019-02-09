/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FIXTURE5_H)
#define      FIXTURE5_H

#include "kernel/ga/problem.h"
#include "kernel/ga/primitive.h"

struct fixture5
{
  fixture5(unsigned n = 4) : prob()
  {
    prob.env.init();

    double v(10.0);
    for (unsigned i(0); i < n; ++i)
    {
      prob.insert(vita::range(-v, +v));
      v *= 10.0;
    }
  }

  vita::de_problem prob;
};

struct fixture5_no_init : fixture5
{
  fixture5_no_init() : fixture5(0) {}
};

#endif  // include guard
