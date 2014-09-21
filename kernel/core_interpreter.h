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
    unsigned penalty() { return penalty_nvi(); }
    bool debug() const { return debug_nvi(); }

  private:
    virtual any run_nvi() = 0;
    virtual unsigned penalty_nvi() = 0;
    virtual bool debug_nvi() const = 0;
  };
}  // namespace vita
#endif  // Include guard
