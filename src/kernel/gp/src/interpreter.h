/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_INTERPRETER_H)
#define      VITA_SRC_INTERPRETER_H

#include "kernel/gp/mep/interpreter.h"

namespace vita
{
/// This is the return type of the src_interpreter::run method. Internal
/// calculations could be performed via different types but the final
/// result is stored in a `number`.
using number = D_DOUBLE;

///
/// \tparam T the type of individual used
///
/// This class extends vita::interpreter to simply manage input variables.
/// For further details see vita::variable class.
///
template<class T>
class src_interpreter : public interpreter<T>
{
public:
  explicit src_interpreter(const T *prg) : interpreter<T>(prg),
                                           example_(nullptr)
  {}

  value_t run(const std::vector<value_t> &);

  [[nodiscard]] value_t fetch_var(unsigned) final;

private:
  // Tells the compiler we want both the run function from interpreter and
  // src_interpreter.
  // Without this statement there will be no `run()` function in the scope
  // of src_interpreter, because it is hidden by another method with the same
  // name (compiler won't search for function in base classes if derived
  // class has at least one method with specified name, even if it has
  // different arguments).
  using interpreter<T>::run;

  const std::vector<value_t> *example_;
};

template<class T> value_t run(const T &, const std::vector<value_t> &);

#include "kernel/gp/src/interpreter.tcc"

}  // namespace vita

#endif  // include guard
