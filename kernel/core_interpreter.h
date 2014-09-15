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

#if !defined(VITA_CORE_INTERPRETER_H)
#define      VITA_CORE_INTERPRETER_H

#include "kernel/any.h"

namespace vita
{
  ///
  /// \brief Minimum interface of an interpreter
  ///
  class core_interpreter
  {
  public:
    any run() { return run_nvi(); }
    bool debug() const { return debug_nvi(); }

  private:
    virtual any run_nvi() = 0;
    virtual bool debug_nvi() const = 0;
  };

  ///
  /// \brief "Run" the individual
  ///
  /// \tparam T the type of individual used
  ///
  /// The interpreter class "executes" an individual (a program) in its
  /// environment.
  ///
  template<class T> class interpreter;
}  // namespace vita
#endif  // Include guard
