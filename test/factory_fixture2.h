/**
 *
 *  \file factory_fixture2.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined FACTORY_FIXTURE2_H
#define      FACTORY_FIXTURE2_H

#include "environment.h"
#include "primitive/factory.h"

struct F_FACTORY2
{
  F_FACTORY2() : env(true)
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY2)");

    vita::symbol_factory &factory(vita::symbol_factory::instance());

    env.insert(factory.make("REAL", {}));
    env.insert(factory.make("FADD", {}));
    env.insert(factory.make("FSUB", {}));
    env.insert(factory.make("FMUL", {}));
    env.insert(factory.make("FIFL", {}));
    env.insert(factory.make("FIFE", {}));
  }

  ~F_FACTORY2()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY2)");
  }

  vita::environment env;
};

#endif  // FACTORY_FIXTURE2_H
