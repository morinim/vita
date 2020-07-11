/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CORE_INTERPRETER_H)
#define      VITA_CORE_INTERPRETER_H

#include "kernel/value.h"

namespace vita
{

///
/// Minimum interface of an interpreter.
///
/// The class "executes" an individual (a program) in its environment. The
/// program can produce an output or perform some actions.
///
/// The class can also check if an individual breaks some constraints
/// assigning a penalty to infeasible individuals.
///
class core_interpreter
{
public:
  value_t run() { return run_nvi(); }
  double penalty() { return penalty_nvi(); }
  bool is_valid() const { return is_valid_nvi(); }

private:
  virtual value_t run_nvi() = 0;
  virtual double penalty_nvi() = 0;
  virtual bool is_valid_nvi() const = 0;
};

}  // namespace vita

#endif  // include guard
