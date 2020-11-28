/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FIXTURE1_H)
#define      FIXTURE1_H

#include "kernel/problem.h"
#include "kernel/gp/src/primitive/factory.h"

struct fixture1
{
  fixture1() : prob(), factory()
  {
    prob.env.init();

    prob.sset.insert(factory.make("REAL", {0}));
    prob.sset.insert(factory.make("FADD", {0}));
    prob.sset.insert(factory.make("FSUB", {0}));
    prob.sset.insert(factory.make("FMUL", {0}));
    prob.sset.insert(factory.make("FDIV", {0}));
    prob.sset.insert(factory.make("FLN", {0}));
    prob.sset.insert(factory.make("FIFL", {0}));
    prob.sset.insert(factory.make("FIFZ", {0}));
    prob.sset.insert(factory.make("FIFE", {0, 0}));
    prob.sset.insert(factory.make("FSIN", {0}));
    prob.sset.insert(factory.make("FABS", {0}));
    prob.sset.insert(factory.make("FLENGTH", {1, 0}));

    prob.sset.insert(factory.make("apple", {1}));
    prob.sset.insert(factory.make("pear", {1}));
    prob.sset.insert(factory.make("grapefruit", {1}));
    prob.sset.insert(factory.make("orange", {1}));
    prob.sset.insert(factory.make("blueberry", {1}));
    prob.sset.insert(factory.make("blackberry", {1}));
    prob.sset.insert(factory.make("passion fruit", {1}));
    prob.sset.insert(factory.make("plum", {1}));
    prob.sset.insert(factory.make("date", {1}));
    prob.sset.insert(factory.make("peach", {1}));
    prob.sset.insert(factory.make("SIFE", {1, 0}));
  }

  vita::problem           prob;
  vita::symbol_factory factory;
};

#endif  // include guard
