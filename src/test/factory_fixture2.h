/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined FACTORY_FIXTURE2_H
#define      FACTORY_FIXTURE2_H

#include "kernel/environment.h"
#include "kernel/symbol_set.h"
#include "kernel/src/primitive/factory.h"

struct F_FACTORY2
{
  F_FACTORY2() : env(&sset, true), factory()
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY2)");

    sset.insert(factory.make("REAL"));
    sset.insert(factory.make("FADD"));
    sset.insert(factory.make("FSUB"));
    sset.insert(factory.make("FMUL"));
    sset.insert(factory.make("FIFL"));
    sset.insert(factory.make("FIFE"));
  }

  ~F_FACTORY2()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY2)");
  }

  vita::environment        env;
  vita::symbol_factory factory;
  vita::symbol_set        sset;
};

#endif  // Include guard
