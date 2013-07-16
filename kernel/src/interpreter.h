/**
 *
 *  \file src/interpreter.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_INTERPRETER_H)
#define      SRC_INTERPRETER_H

#include "kernel/interpreter.h"

namespace vita
{
  ///
  /// This class extends vita::interpreter to simply manage input variables.
  /// For further details see also src_variable.
  ///
  class src_interpreter : public interpreter
  {
  public:
    explicit src_interpreter(const individual &ind) : interpreter(ind) {}

    any run(const std::vector<any> &ex)
    { example_ = &ex; return interpreter::run(); }

    any fetch_var(unsigned i)
    { assert(i < example_->size()); return (*example_)[i]; }

  private:
    const std::vector<any> *example_;
  };
}  // namespace vita

#endif  // SRC_INTERPRETER_H
