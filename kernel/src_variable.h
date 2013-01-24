/**
 *
 *  \file src_variable.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_VARIABLE_H)
#define      SRC_VARIABLE_H

#include <boost/variant.hpp>

#include "data.h"
#include "src_interpreter.h"
#include "terminal.h"

namespace vita
{
  ///
  /// A variable is an input argument (feature) for a symbolic regression or
  /// classification problem.
  ///
  class variable : public terminal
  {
  public:
    variable(const std::string &name, size_t v, category_t t = 0)
      : terminal(name, t, true), var_(v) {}

    ///
    /// \return the value of the variable (as a \c any).
    ///
    /// \note
    /// the method requires a src_interpreter to work.
    ///
    any eval(interpreter *i) const
    {
      return static_cast<src_interpreter *>(i)->eval_var(var_);
    }

  private:  // Private data members.
    size_t var_;
  };
}  // namespace vita
#endif  // SRC_VARIABLE_H
