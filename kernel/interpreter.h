/**
 *
 *  \file interpreter.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(INTERPRETER_H)
#define      INTERPRETER_H

#include <boost/optional.hpp>

#include "matrix.h"

namespace vita
{
  class individual;

  class interpreter
  {
  public:
    explicit interpreter(const individual &, interpreter *const = 0);

    any run();
    any run(const locus &);

    any eval();
    any eval(size_t);
    any eval_adf_arg(size_t);

    bool debug() const;

    static double to_double(const any &);
    static std::string to_string(const any &);

  private:
    // Instruction pointer.
    locus ip_;

    interpreter *const context_;

    const individual &ind_;

    mutable matrix<boost::optional<any>> cache_;
  };

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///
}  // namespace vita

#endif  // INTERPRETER_H
