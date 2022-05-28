/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CORE_INTERPRETER_H)
#define      VITA_CORE_INTERPRETER_H

#include "kernel/common.h"
#include "kernel/value.h"
#include "kernel/vitafwd.h"

namespace vita
{

class symbol_params
{
public:
  // Return value could be ignored. E.g. the caller is only interested in the
  // side effects of the call (typically agent simulation).
  virtual value_t fetch_arg(unsigned) = 0;

  [[nodiscard]] value_t operator[](unsigned i) { return fetch_arg(i); }

  [[nodiscard]] virtual terminal_param_t fetch_param() const = 0;

  [[nodiscard]] virtual value_t fetch_var(unsigned) { return {}; }
};

///
/// Minimum interface of an interpreter.
///
/// The class "executes" an individual (a program). The program can produce an
/// output or perform some actions.
///
/// The class can also check if an individual breaks some constraints assigning
/// a penalty to infeasible individuals.
///
class core_interpreter : public symbol_params
{
public:
  value_t run() { return run_nvi(); }
  [[nodiscard]] double penalty() { return penalty_nvi(); }
  [[nodiscard]] bool is_valid() const { return is_valid_nvi(); }

private:
  virtual value_t run_nvi() = 0;
  virtual double penalty_nvi() = 0;
  virtual bool is_valid_nvi() const = 0;
};

///
/// A handy short-cut for one-time execution of an individual.
///
/// \param[in] ind individual/program to be run
/// \return        possible output value of the individual
///
template<class T> value_t run(const T &ind)
{
  return interpreter<T>(&ind).run();
}

}  // namespace vita

#endif  // include guard
