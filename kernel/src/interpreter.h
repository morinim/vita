/**
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(SRC_INTERPRETER_H)
#define      SRC_INTERPRETER_H

#include "kernel/interpreter.h"

namespace vita
{
  ///
  /// \tparam T the type of individual used.
  ///
  /// This class extends vita::interpreter to simply manage input variables.
  /// For further details see also src_variable.
  ///
  template<class T>
  class src_interpreter : public interpreter<T>
  {
  public:
    explicit src_interpreter(const T &prg, interpreter<T> *ctx = nullptr)
      : interpreter<T>(prg, ctx)
    {}

    // Tells the compiler we want both the run function from interpreter and
    // src_interpreter.
    // Without this statement there will be no any run() function in the scope
    // of src_interpreter, because it is hidden by another method with the same
    // name (compiler won't search for function in base classes if derived
    // class has at least one method with specified name, even if it has
    // different arguments).
    using interpreter<T>::run;

    any run(const std::vector<any> &);

    any fetch_var(unsigned);

  private:
    const std::vector<any> *example_;
  };

#include "kernel/src/interpreter_inl.h"
}  // namespace vita

#endif  // SRC_INTERPRETER_H
