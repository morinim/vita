/**
 *
 *  \file factory_fixture1.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined FACTORY_FIXTURE1_H
#define      FACTORY_FIXTURE1_H

#include "environment.h"
#include "primitive/factory.h"

struct F_FACTORY1
{
  F_FACTORY1() : env(true)
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY1)");

    vita::symbol_factory &factory(vita::symbol_factory::instance());

    env.insert(factory.make("REAL", {0}));
    env.insert(factory.make("FADD", {0}));
    env.insert(factory.make("FSUB", {0}));
    env.insert(factory.make("FMUL", {0}));
    env.insert(factory.make("FDIV", {0}));
    env.insert(factory.make("FLN", {0}));
    env.insert(factory.make("FIFL", {0}));
    env.insert(factory.make("FIFZ", {0}));
    env.insert(factory.make("FIFE", {0, 0}));
    env.insert(factory.make("FSIN", {0}));
    env.insert(factory.make("FABS", {0}));
    env.insert(factory.make("FLENGTH", {1, 0}));

    env.insert(factory.make("apple", {1}));
    env.insert(factory.make("pear", {1}));
    env.insert(factory.make("grapefruit", {1}));
    env.insert(factory.make("orange", {1}));
    env.insert(factory.make("blueberry", {1}));
    env.insert(factory.make("blackberry", {1}));
    env.insert(factory.make("passion fruit", {1}));
    env.insert(factory.make("plum", {1}));
    env.insert(factory.make("date", {1}));
    env.insert(factory.make("peach", {1}));
    env.insert(factory.make("IFE", {1, 0}));
  }

  ~F_FACTORY1()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY1)");
  }

  vita::environment env;
};

#endif  // FACTORY_FIXTURE1_H
