/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MEP_INTERPRETER_H)
#define      VITA_MEP_INTERPRETER_H

#include "kernel/core_interpreter.h"
#include "kernel/gp/function.h"
#include "kernel/gp/gene.h"
#include "kernel/vitafwd.h"
#include "utility/matrix.h"

namespace vita
{
///
/// A specialization of the core_interpreter class.
///
/// \note
/// This is an example of dependency injection via constructor injection: the
/// class gets the dependencies as an argument to its constructor. It saves
/// the reference to dependent classes as private variables.
/// All the dependencies are truly visible with constructor signature and
/// cyclic dependencies don't happen because of the well-defined order of
/// instantiation.
///
/// \see
/// - https://en.wikipedia.org/wiki/Dependency_injection
/// - https://stackoverflow.com/q/4542789/3235496
/// - https://stackoverflow.com/q/12387239/3235496
/// - https://stackoverflow.com/q/1974682/3235496
///
template<>
class interpreter<i_mep> : public core_interpreter
{
public:
  explicit interpreter(const i_mep *);

  [[nodiscard]] index_t fetch_index(unsigned) const;

  [[nodiscard]] const i_mep &program() const { return *prg_; }

  [[nodiscard]] terminal_param_t fetch_param() const final;
  value_t fetch_arg(unsigned) final;

private:
  // *** Private support methods ***
  value_t run_locus(const locus &);
  [[nodiscard]] double penalty_locus(const locus &);

  // Nonvirtual interface.
  value_t run_nvi() override;
  [[nodiscard]] double penalty_nvi() override;
  [[nodiscard]] bool is_valid_nvi() const override;

  // *** Private data members ***
  const i_mep *prg_;

  struct elem_ {bool valid; value_t value;};
  mutable matrix<elem_> cache_;

  // Instruction pointer.
  locus ip_;
};

}  // namespace vita

#endif  // include guard
