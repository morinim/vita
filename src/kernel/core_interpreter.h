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

///
/// An interface for parameter passing to functions / terminals.
///
/// symbol_params is a subset of the core_interpreter interface. Coding a user
/// defined function is more natural when you can think the input parameters as
/// a simple array of values.
/// Actually things are more complex but user doesn't have to know. The only
/// detail you must remember is that parameters are lazy evaluated so:
/// - store the value of `fetch_arg(i)` (i.e. `operator[](i)`) in a local
///   variable for multiple uses;
/// - call `fetch_arg(i)` only if you need the `i`-th argument.
///
class symbol_params
{
public:
  /// Fetches a specific input parameter assuming referential transparency.
  /// Referential transparency allows cache based optimization for argument
  /// retrieval. If this kind of optimization isn't required the implementation
  /// can be a simple call to `fetch_upaque_arg`.
  [[nodiscard]] virtual value_t fetch_arg(unsigned) = 0;

  /// Fetches a specific input parameter without assuming referential
  /// transparency.
  /// \remark
  /// Sometimes return value is ignored: typically for agent simulation (the
  /// caller is only interested in the side effects of the call).
  virtual value_t fetch_opaque_arg(unsigned) = 0;

  /// Equivalent to fetch_arg().
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
