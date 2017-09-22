/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined FACTORY_FIXTURE1_H
#define      FACTORY_FIXTURE1_H

#include "kernel/environment.h"
#include "kernel/symbol_set.h"
#include "kernel/src/primitive/factory.h"

struct F_FACTORY1
{
  F_FACTORY1() : env(&sset, vita::initialization::standard), factory()
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY1)");

    sset.insert(factory.make("REAL", {0}));
    sset.insert(factory.make("FADD", {0}));
    sset.insert(factory.make("FSUB", {0}));
    sset.insert(factory.make("FMUL", {0}));
    sset.insert(factory.make("FDIV", {0}));
    sset.insert(factory.make("FLN", {0}));
    sset.insert(factory.make("FIFL", {0}));
    sset.insert(factory.make("FIFZ", {0}));
    sset.insert(factory.make("FIFE", {0, 0}));
    sset.insert(factory.make("FSIN", {0}));
    sset.insert(factory.make("FABS", {0}));
    sset.insert(factory.make("FLENGTH", {1, 0}));

    sset.insert(factory.make("apple", {1}));
    sset.insert(factory.make("pear", {1}));
    sset.insert(factory.make("grapefruit", {1}));
    sset.insert(factory.make("orange", {1}));
    sset.insert(factory.make("blueberry", {1}));
    sset.insert(factory.make("blackberry", {1}));
    sset.insert(factory.make("passion fruit", {1}));
    sset.insert(factory.make("plum", {1}));
    sset.insert(factory.make("date", {1}));
    sset.insert(factory.make("peach", {1}));
    sset.insert(factory.make("SIFE", {1, 0}));
  }

  ~F_FACTORY1()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY1)");
  }

  vita::environment        env;
  vita::symbol_factory factory;
  vita::symbol_set        sset;
};

#endif  // include guard
