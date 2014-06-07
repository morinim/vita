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

#if !defined FACTORY_FIXTURE5_H
#define      FACTORY_FIXTURE5_H

#include "kernel/environment.h"
#include "kernel/symbol_set.h"
#include "kernel/ga/primitive.h"

struct F_FACTORY5
{
  F_FACTORY5() : env(true)
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY5)");

    sset.insert(vita::ga::parameter(0,    -10.0,    10.0));
    sset.insert(vita::ga::parameter(1,   -100.0,   100.0));
    sset.insert(vita::ga::parameter(2,  -1000.0,  1000.0));
    sset.insert(vita::ga::parameter(3, -10000.0, 10000.0));
  }

  ~F_FACTORY5()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY5)");
  }

  vita::environment env;
  vita::symbol_set sset;
};

#endif  // Include guard
