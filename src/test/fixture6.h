/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FIXTURE6_H)
#define      FIXTURE6_H

#include "kernel/ga/problem.h"
#include "kernel/ga/primitive.h"

struct fixture6
{
  fixture6(unsigned n = 4) : prob(), ranges()
  {
    prob.env.init();

    int v(10);
    for (unsigned i(0); i < n; ++i)
    {
      const auto r(vita::range(-v, +v));
      prob.insert(r);
      ranges.push_back(r);

      v *= 10;
    }
  }

  vita::ga_problem prob;
  std::vector<vita::range_t<int>> ranges;
};

struct fixture6_no_init : fixture6
{
  fixture6_no_init() : fixture6(0) {}
};

#endif  // include guard
