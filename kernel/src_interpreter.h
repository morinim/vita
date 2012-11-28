/**
 *
 *  \file src_interpreter.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_INTERPRETER_H)
#define      SRC_INTERPRETER_H

#include "interpreter.h"
#include "src_variable.h"

namespace vita
{
  class src_interpreter : public interpreter
  {
  public:
    src_interpreter(const individual &ind, std::vector<variable_ptr> *v)
      : interpreter(ind), variables_(v)
    {}

    any run(const data::example &ex)
    { load_vars(ex); return interpreter::operator()(); }

  private:
    void load_vars(const data::example &);

    std::vector<variable_ptr> *variables_;
  };
}  // namespace vita

#endif  // SRC_INTERPRETER_H
