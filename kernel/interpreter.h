/**
 *
 *  \file kernel/interpreter.h
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

#include "kernel/matrix.h"

namespace vita
{
  class individual;
  template<class T> class interpreter;

  template<>
  class interpreter<individual>
  {
  public:
    explicit interpreter(const individual &,
                         interpreter<individual> * = nullptr);

    any run();

    any fetch_param();
    any fetch_arg(unsigned);
    any fetch_adf_arg(unsigned);

    bool debug() const;

    static double to_double(const any &);
    static std::string to_string(const any &);

  private:  // Private methods.
    any run(const locus &);

  private:
    // Instruction pointer.
    locus ip_;

    interpreter<individual> *const context_;

    const individual &ind_;

    mutable matrix<boost::optional<any>> cache_;
  };

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///
}  // namespace vita

#endif  // INTERPRETER_H
