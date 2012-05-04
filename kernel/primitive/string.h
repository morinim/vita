/**
 *
 *  \file string.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(STRING_PRIMITIVE_H)
#define      STRING_PRIMITIVE_H

#include <boost/any.hpp>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita
{
  namespace str
  {
    class ife : public function
    {
    public:
      explicit ife(category_t t1, category_t t2)
        : function("SIFE", t2, {t1, t1, t2, t2}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any v0(i->eval(0));
        if (v0.empty())  return v0;

        const boost::any v1(i->eval(1));
        if (v1.empty())  return v1;

        if (boost::any_cast<std::string>(v0) ==
            boost::any_cast<std::string>(v1))
          return i->eval(2);
        else
          return i->eval(3);
      }
    };
  }  // namespace str
}  // namespace vita

#endif  // STRING_PRIMITIVE_H
