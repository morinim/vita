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

#if !defined(FIXTURE2_H)
#define      FIXTURE2_H

#include "kernel/problem.h"
#include "kernel/gp/src/primitive/factory.h"

struct fixture2
{
  fixture2() : prob(), factory()
  {
    prob.env.init();

    prob.sset.insert(factory.make("REAL"));
    prob.sset.insert(factory.make("FADD"));
    prob.sset.insert(factory.make("FSUB"));
    prob.sset.insert(factory.make("FMUL"));
    prob.sset.insert(factory.make("FIFL"));
    prob.sset.insert(factory.make("FIFE"));
  }

  vita::problem           prob;
  vita::symbol_factory factory;
};

#endif  // include guard
