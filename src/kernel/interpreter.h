/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INTERPRETER_H)
#define      VITA_INTERPRETER_H

#include "kernel/core_interpreter.h"
#include "kernel/function.h"
#include "kernel/gene.h"
#include "kernel/vitafwd.h"
#include "utility/matrix.h"

namespace vita
{
///
/// A template specialization of the core_interpreter class.
///
/// \tparam T the type of individual used
///
/// \note
/// This class would like to be a generic `interpreter<T>` implementation,
/// but interpreter and individual are strongly coupled objects: the
/// interpreter must be build around the peculiarities of the specific
/// individual class. So don't expect to use this template for `T` different
/// from `i_mep` without some modifications.
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
/// - <http://en.wikipedia.org/wiki/Dependency_injection>
/// - <http://stackoverflow.com/q/4542789/3235496>
/// - <http://stackoverflow.com/q/12387239/3235496>
/// - <http://stackoverflow.com/q/1974682/3235496>
///
template<class T>
class interpreter : public core_interpreter
{
public:
  explicit interpreter(const T *, interpreter * = nullptr);

  terminal::param_t fetch_param();
  value_t fetch_arg(unsigned);
  value_t fetch_adf_arg(unsigned);
  index_t fetch_index(unsigned) const;

  const T &program() const { return *prg_; }

private:
  // *** Private support methods ***
  value_t run_locus(const locus &);
  double penalty_locus(const locus &);

  // Nonvirtual interface.
  value_t run_nvi() override;
  double penalty_nvi() override;
  bool is_valid_nvi() const override;

  // *** Private data members ***
  const T *prg_;

  struct elem_ {bool valid; value_t value;};
  mutable matrix<elem_> cache_;

  // Instruction pointer.
  locus ip_;

  // This is a pointer since we need to describe a one-or-zero relationship.
  interpreter *context_;
};

template<class T> value_t run(const T &);

#include "kernel/interpreter.tcc"

}  // namespace vita

#endif  // include guard
