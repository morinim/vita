/**
 *
 *  \file string.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
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
    class length : public function
    {
    public:
      explicit length(category_t t1, category_t t2)
        : function("LENGTH", t2, {t1}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev(i->eval(0));
        if (ev.empty())  return ev;

        return boost::any_cast<std::string>(ev).size();
      }
    };

    class ife : public function
    {
    public:
      explicit ife(category_t t1, category_t t2)
        : function("IFE", t2, {t1, t1, t2, t2}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        if (boost::any_cast<std::string>(ev0) ==
            boost::any_cast<std::string>(ev1))
          return i->eval(2);
        else
          return i->eval(3);
      }
    };
  }  // namespace str
}  // namespace vita

#endif  // STRING_PRIMITIVE_H
