/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_VARIABLE_H)
#define      VITA_SRC_VARIABLE_H

#include "kernel/data.h"
#include "kernel/src/interpreter.h"
#include "kernel/terminal.h"

namespace vita
{
  ///
  /// A variable is an input argument (feature) for a symbolic regression or
  /// classification problem.
  ///
  class variable : public terminal
  {
  public:
    variable(const std::string &name, unsigned var_id, category_t t = 0)
      : terminal(name, t, true), var_(var_id)
    {}

    ///
    /// \return the value of the variable (as a \c any).
    ///
    /// \note
    /// the method requires a src_interpreter to work.
    ///
    any eval(interpreter<i_mep> *i) const
    {
      return static_cast<src_interpreter<i_mep> *>(i)->fetch_var(var_);
    }

  private:  // Private data members.
    unsigned var_;
  };
}  // namespace vita

#endif  // Include guard
