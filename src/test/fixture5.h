/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FIXTURE5_H)
#define      FIXTURE5_H

#include "kernel/problem.h"
#include "kernel/ga/primitive.h"

struct fixture5
{
  fixture5(unsigned n = 4) : prob(vita::initialization::standard)
  {
    double v(10.0);

    for (unsigned i(0); i < n; ++i)
    {
      prob.sset.insert<vita::ga::real>(vita::range(-v, +v));
      v *= 10.0;
    }
  }

  vita::problem prob;
};

struct fixture5_no_init : fixture5
{
  fixture5_no_init() : fixture5(0) {}
};

#endif  // include guard
