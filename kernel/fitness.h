/**
 *
 *  \file fitness.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(FITNESS_H)
#define      FITNESS_H

#include "kernel/vita.h"

namespace vita
{
  ///
  /// A value assigned to an individual which reflects how well the individual
  /// solves the task.
  ///
  typedef double fitness_t;

  inline double distance(fitness_t f1, fitness_t f2) {return std::fabs(f1-f2);}
}  // namespace vita

#endif  // FITNESS_H
