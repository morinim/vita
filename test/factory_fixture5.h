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
  F_FACTORY5() : env(true), sset()
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY5)");
  }

  ~F_FACTORY5()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY5)");
  }

  vita::environment env;
  vita::symbol_set sset;
};

#endif  // Include guard
