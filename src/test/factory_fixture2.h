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

#if !defined FACTORY_FIXTURE2_H
#define      FACTORY_FIXTURE2_H

#include "kernel/problem.h"
#include "kernel/src/primitive/factory.h"

struct F_FACTORY2
{
  F_FACTORY2() : prob(vita::initialization::standard), factory()
  {
    BOOST_TEST_MESSAGE("Setup fixture (FACTORY2)");

    prob.sset.insert(factory.make("REAL"));
    prob.sset.insert(factory.make("FADD"));
    prob.sset.insert(factory.make("FSUB"));
    prob.sset.insert(factory.make("FMUL"));
    prob.sset.insert(factory.make("FIFL"));
    prob.sset.insert(factory.make("FIFE"));
  }

  ~F_FACTORY2()
  {
    BOOST_TEST_MESSAGE("Teardown fixture (FACTORY2)");
  }

  vita::problem           prob;
  vita::symbol_factory factory;
};

#endif  // include guard
