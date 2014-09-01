/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_INTERPRETER_H)
#define      VITA_GA_INTERPRETER_H

#include "kernel/core_interpreter.h"
#include "kernel/gene.h"
#include "kernel/vitafwd.h"

namespace vita
{
  /// Pointer to multivariable real function.
  using ga_function = double (*)(const std::vector<double> &);

  ///
  /// \brief A template specialization for interpreter<T> class
  ///
  /// It's similar to a GP interpreter, but this also stores a pointer to
  /// a multivariable real function.
  ///
  /// When the interpreter is executed, the parameters inside the current
  /// individual are passed as arguments of the function and the output value
  /// is the result.
  ///
  template<>
  class interpreter<i_ga> : public core_interpreter
  {
  public:
    /// A multivariable real function.
    static ga_function function;

  public:
    explicit interpreter(const i_ga &);

    virtual any run() override;

    gene::param_type fetch_param(unsigned);

    virtual bool debug() const override;

  private:
    const i_ga &ind_;
  };
}  // namespace vita

#endif  // Include guard
