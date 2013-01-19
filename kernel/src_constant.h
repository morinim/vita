/**
 *
 *  \file src_constant.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_CONSTANT_H)
#define      SRC_CONSTANT_H

#include <boost/variant.hpp>

#include "data.h"
#include "terminal.h"

namespace vita
{
  class constant : public terminal
  {
    //private:
    // C++11 allows constructors to call other peer constructors (known as
    // delegation). This allows constructors to utilize another constructor's
    // behavior with a minimum of added code.
    //constant(const std::string &name, const any &c, category_t = 0)
    //  : terminal(name, t, false, false, default_weight*2), val(c) {}
  public:
    explicit constant(bool c, category_t t = 0)
      : terminal(boost::lexical_cast<std::string>(c), t, false, false,
                 default_weight * 2), val(c) {}
    explicit constant(double c, category_t t = 0)
      : terminal(boost::lexical_cast<std::string>(c), t, false, false,
                 default_weight * 2), val(c) {}
    explicit constant(int c, category_t t = 0)
      : terminal(boost::lexical_cast<std::string>(c), t, false, false,
                 default_weight * 2), val(c) {}
    explicit constant(const std::string &c, category_t t = 0)
      : terminal("\"" + c + "\"", t, false, false, default_weight * 2),
        val(c) {}

    ///
    /// \return the value of the constant (as a \c any).
    ///
    /// The argument is not used: the value of a constant is stored within the
    /// object and we don't need an \c interpreter to discover it.
    ///
    any eval(vita::interpreter *) const
    {
      switch (val.which())
      {
      case 0:  return any(boost::get<bool>(val));
      case 1:  return any(boost::get<int>(val));
      case 2:  return any(boost::get<double>(val));
      default: return any(boost::get<std::string>(val));
      }
    }

  private:  // Private data members.
    data::example::value_t val;
  };
}  // namespace vita
#endif // SRC_CONSTANT_H
