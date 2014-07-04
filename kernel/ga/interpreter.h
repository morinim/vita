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
#include "kernel/vitafwd.h"

namespace vita
{
  /// Pointer to multivariable real function.
using ga_function = double (*)(const std::vector<double> &);

  ///
  /// \brief A template specialization for interpreter<T> class
  ///
  /// \note
  /// We don't have a generic interpreter<T> implementation (e.g. see the
  /// lambda_f source code) because interpreter and individual are strongly
  /// coupled: the interpreter must be build around the peculiarities of
  /// the specific individual class.
  ///
  template<>
  class interpreter<i_num_ga> : public core_interpreter
  {
  public:
    /// A multivariable real function.
    static ga_function function;

  public:
    explicit interpreter(const i_num_ga &);

    virtual any run() override;

    any fetch_param(unsigned);

    virtual bool debug() const override;

  private:
    const i_num_ga &ind_;
  };
}  // namespace vita

#endif  // Include guard
