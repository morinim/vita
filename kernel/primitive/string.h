/**
 *
 *  \file string.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(STRING_PRIMITIVE_H)
#define      STRING_PRIMITIVE_H

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>

#include "function.h"
#include "interpreter.h"
#include "random.h"
#include "terminal.h"

namespace vita
{
  namespace str
  {
    class ife : public function
    {
    public:
      explicit ife(category_t t1, category_t t2)
        : function("SIFE", t2, {t1, t1, t2, t2}) {}

      any eval(interpreter *i) const
      {
        const any v0(i->eval(0));
        if (v0.empty())  return v0;

        const any v1(i->eval(1));
        if (v1.empty())  return v1;

        if (any_cast<std::string>(v0) == any_cast<std::string>(v1))
          return i->eval(2);
        else
          return i->eval(3);
      }
    };
  }  // namespace str
}  // namespace vita

#endif  // STRING_PRIMITIVE_H
