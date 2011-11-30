/**
 *
 *  \file int.h
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

#if !defined(INT_PRIMITIVE_H)
#define      INT_PRIMITIVE_H

#include <boost/any.hpp>

#include <limits>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita
{
  /// Integer overflow is undefined behavior. This means that implementations
  /// have a great deal of latitude in how they deal with signed integer
  /// overflow. An implementation that defines signed integer types as being
  /// modulo, for example, need not detect integer overflow. Implementations
  /// may also trap on signed arithmetic overflows, or simply assume that
  /// overflows will never happen and generate object code accordingly. For
  /// these reasons, it is important to ensure that operations on signed
  /// integers do no result in signed overflow
  namespace integer
  {
    ///
    /// Integer ephemeral random constant.
    /// \see dbl::number
    ///
    class number : public terminal
    {
    public:
      explicit number(category_t t, int m = -128, int u = 127)
        : terminal("NUM", t, false, true, default_weight*2), min(m), upp(u) {}

      int init() const { return random::between<int>(min, upp); }

      std::string display(int v) const
      { return boost::lexical_cast<std::string>(v); }

      boost::any eval(interpreter *i) const
      { return boost::any_cast<int>(i->eval()); }

    private:
      const int min, upp;
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Addition
    class add : public function
    {
    public:
      explicit add(category_t t)
        : function("ADD", t, 2, function::default_weight, true) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v0 > 0 && v1 > 0 && (v0 > std::numeric_limits<int>::max() - v1))
          return std::numeric_limits<int>::max();
        if (v0 < 0 && v1 < 0 && (v0 < std::numeric_limits<int>::min() - v1))
          return std::numeric_limits<int>::min();

        return v0 + v1;
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Division
    class div : public function
    {
    public:
      explicit div(category_t t) : function("DIV", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v1 == 0 || (v0 == std::numeric_limits<int>::min() && (v1 == -1)))
          return v0;
        else
          return v1 / v0;
      }
    };

    class ife : public function
    {
    public:
      explicit ife(category_t t1, category_t t2)
        : function("IFE", t2, {t1, t1, t2, t2}) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v0 == v1)
          return i->eval(2);
        else
          return i->eval(3);
      }
    };

    class ifl : public function
    {
    public:
      explicit ifl(category_t t1, category_t t2)
        : function("IFL", t2, {t1, t1, t2, t2}) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v0 < v1)
          return i->eval(2);
        else
          return i->eval(3);
      }
    };

    class ifz : public function
    {
    public:
      explicit ifz(category_t t) : function("IFZ", t, 3) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));

        if (v0 == 0)
          return i->eval(1);
        else
          return i->eval(2);
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Modulo
    class mod : public function
    {
    public:
      explicit mod(category_t t) : function("MOD", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v1 == 0 || (v0 == std::numeric_limits<int>::min() && (v1 == -1)))
          return v1;
        else
          return v0 % v1;
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Multiplication
    class mul : public function
    {
    public:
      explicit mul(category_t t)
        : function("MUL", t, 2, function::default_weight, true) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v0 > 0)
          if (v1 > 0)
          {
            assert(v0 > 0 && v1 > 0);
            if (v0 > std::numeric_limits<int>::max() / v1)
              return std::numeric_limits<int>::max();
          }
          else  // v1 is non-positive
          {
            assert(v0 > 0 && v1 <= 0);
            if (v1 < std::numeric_limits<int>::min() / v0)
              return std::numeric_limits<int>::min();
          }
        else  // v0 is non-positive
          if (v1 > 0)
          {
            assert(v0 <= 0 && v1 > 0);
            if (v0 < std::numeric_limits<int>::min() / v1)
              return std::numeric_limits<int>::min();
          }
          else  // v1 is non-positive
          {
            assert(v0 <= 0 && v1 <= 0);
            if (v0 != 0 && v1 < std::numeric_limits<int>::max() / v0)
              return std::numeric_limits<int>::max();
          }

        return v0 * v1;
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-LeftShiftOperator
    class shl : public function
    {
    public:
      explicit shl(category_t t) : function("SHL", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v0 < 0 || v1 < 0 ||
            v1 >= static_cast<int>(sizeof(int) * CHAR_BIT) ||
            v0 > std::numeric_limits<int>::max() >> v1)
          return v0;

        return v0 << v1;
      }
    };

    /// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Subtraction
    class sub : public function
    {
    public:
      explicit sub(category_t t) : function("SUB", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const int v0(boost::any_cast<int>(i->eval(0)));
        const int v1(boost::any_cast<int>(i->eval(1)));

        if (v0 < 0 && v1 > 0 && (v0 < std::numeric_limits<int>::min() + v1))
          return std::numeric_limits<int>::min();
        if (v0 > 0 && v1 < 0 && (v0 > std::numeric_limits<int>::max() + v1))
          return std::numeric_limits<int>::max();

        return v0 - v1;
      }
    };
  }  // namespace integer
}  // namespace vita

#endif  // INT_PRIMITIVE_H
