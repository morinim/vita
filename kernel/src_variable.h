/**
 *
 *  \file src_variable.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_VARIABLE_H)
#define      SRC_VARIABLE_H

#include <boost/variant.hpp>

#include "kernel/data.h"
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
    explicit variable(const std::string &name, category_t t = 0)
      : terminal(name, t, true) {}

    ///
    /// \return the value of the variable (as a \c boost::any).
    ///
    /// The argument is not used: the value of a variable is stored within the
    /// object and we don't need an \c interpreter to discover it.
    ///
    boost::any eval(vita::interpreter *) const
    {
      switch (val.which())
      {
      case 0:  return boost::any(boost::get<bool>(val));
      case 1:  return boost::any(boost::get<int>(val));
      case 2:  return boost::any(boost::get<double>(val));
      default: return boost::any(boost::get<std::string>(val));
      }
    }

  public:  // Data members.
    data::example::value_t val;
  };
}  // namespace vita
#endif  // SRC_VARIABLE_H
