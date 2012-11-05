/**
 *
 *  \file interpreter.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(INTERPRETER_H)
#define      INTERPRETER_H

#include <boost/any.hpp>
#include <boost/multi_array.hpp>
#include <boost/optional.hpp>

#include "locus.h"

namespace vita
{
  class individual;

  class interpreter
  {
  public:
    explicit interpreter(const individual &, interpreter *const = 0);

    boost::any operator()();
    boost::any operator()(const locus &);

    boost::any eval();
    boost::any eval(unsigned);
    boost::any eval_adf_arg(unsigned);

    bool check() const;

    static double to_double(const boost::any &);
    static std::string to_string(const boost::any &);

  private:
    // Instruction pointer.
    locus ip_;

    interpreter *const context_;

    const individual &ind_;

    mutable boost::multi_array<boost::optional<boost::any>, 2> cache_;
  };

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///
}  // namespace vita

#endif  // INTERPRETER_H
