/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined FACTORY_FIXTURE5_H
#define      FACTORY_FIXTURE5_H

#include "kernel/problem.h"
#include "kernel/ga/primitive.h"

struct F_FACTORY5
{
  F_FACTORY5(unsigned n = 4) : prob(vita::initialization::standard)
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY5)");

    double v(10.0);

    for (unsigned i(0); i < n; ++i)
    {
      prob.sset.insert<vita::ga::real>(vita::range(-v, +v));
      v *= 10.0;
    }
  }

  ~F_FACTORY5()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY5)");
  }

  vita::problem prob;
};

struct F_FACTORY5_NO_INIT : F_FACTORY5
{
  F_FACTORY5_NO_INIT() : F_FACTORY5(0) {}
};

#endif  // include guard
