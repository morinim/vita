/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CORE_INTERPRETER_H)
#define      VITA_CORE_INTERPRETER_H

#include "kernel/any.h"

namespace vita
{
///
/// \brief Minimum interface of an interpreter
///
/// The class "executes" an individual (a program) in its environment. The
/// program can produce an output or perform some action.
///
/// The class can also check if an individual breaks some constraints
/// assigning a penalty to infeasible individuals.
///
class core_interpreter
{
public:
  any run() { return run_nvi(); }
  double penalty() { return penalty_nvi(); }
  bool debug(const void *p = nullptr) const { return debug_nvi(p); }

private:
  virtual any run_nvi() = 0;
  virtual double penalty_nvi() = 0;
  virtual bool debug_nvi(const void *) const = 0;
};

}  // namespace vita

#endif  // Include guard
