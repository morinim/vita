/**
 *
 *  \file sr_pri.h
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

#if !defined(SR_PRIMITIVE_H)
#define      SR_PRIMITIVE_H

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
  namespace sr
  {
    class variable : public terminal
    {
    public:
      variable(const std::string &name, symbol_t t = sym_real)
        : terminal(name, t, true) {}

      boost::any eval(vita::interpreter *) const
      { return is_bad(val) ? boost::any() : val; }

      double val;
    };

    class constant : public terminal
    {
    public:
      constant(double c, symbol_t t = sym_real)
        : terminal("CONST", t, false, false, default_weight*2), val(c) {}

      std::string display() const
      {
        std::ostringstream s;
        s << val;
        return s.str();
      }

      boost::any eval(interpreter *) const { return val; }

      const double val;
    };

    class number : public terminal
    {
    public:
      number(int m, int u, symbol_t t = sym_real)
        : terminal("NUM", t, false, true, default_weight*2), min(m), upp(u) {}

      int init() const { return random::between<int>(min, upp); }

      std::string display(int v) const
      {
        std::ostringstream s;
        s << static_cast<double>(v);
        return s.str();
      }

      boost::any eval(interpreter *i) const
      { return static_cast<double>(boost::any_cast<int>(i->eval())); }

    private:
      const int min, upp;
    };

    class abs : public function
    {
    public:
      explicit abs(symbol_t t = sym_real) : function("ABS", t, 1) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev(i->eval(0));
        if (ev.empty())  return ev;

        return std::fabs(boost::any_cast<double>(ev));
      }
    };

    class add : public function
    {
    public:
      explicit add(symbol_t t = sym_real)
        : function("ADD", t, 2, function::default_weight, true) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(boost::any_cast<double>(ev0) +
                         boost::any_cast<double>(ev1));
        if (isinf(ret))  return boost::any();

        return ret;
      }
    };

    class div : public function
    {
    public:
      explicit div(symbol_t t = sym_real) : function("DIV", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(boost::any_cast<double>(ev0) /
                         boost::any_cast<double>(ev1));
        if (is_bad(ret))  return boost::any();

        return ret;
      }
    };

    class idiv : public function
    {
    public:
      explicit idiv(symbol_t t = sym_real) : function("IDIV", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(std::floor(boost::any_cast<double>(ev0) /
                                    boost::any_cast<double>(ev1)));
        if (is_bad(ret))  return boost::any();

        return ret;
      }
    };

    class ife : public function
    {
    public:
      explicit ife(symbol_t t = sym_real) : function("IFE", t, 4) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double cmp(std::fabs(boost::any_cast<double>(ev0) -
                                   boost::any_cast<double>(ev1)));

        if (cmp < float_epsilon)
          return i->eval(2);
        else
          return i->eval(3);
      }
    };

    class ifl : public function
    {
    public:
      explicit ifl(symbol_t t = sym_real) : function("IFL", t, 4) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        if ( boost::any_cast<double>(ev0) < boost::any_cast<double>(ev1) )
          return i->eval(2);
        else
          return i->eval(3);
      }
    };

    class ifz : public function
    {
    public:
      explicit ifz(symbol_t t = sym_real) : function("IFZ", t, 3) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        if (std::fabs(boost::any_cast<double>(ev0)) < float_epsilon)
          return i->eval(1);
        else
          return i->eval(2);
      }
    };

    class ln : public function
    {
    public:
      explicit ln(symbol_t t = sym_real)
        : function("LN", t, 1, function::default_weight/2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const double ret(std::log(boost::any_cast<double>(i->eval(0))));
        if (is_bad(ret))  return boost::any();

        return ret;
      }
    };

    class mod : public function
    {
    public:
      explicit mod(symbol_t t = sym_real) : function("MOD", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(std::fmod(boost::any_cast<double>(ev0),
                                   boost::any_cast<double>(ev1)));
        if (is_bad(ret))  return boost::any();

        return ret;
      }
    };

    class mul : public function
    {
    public:
      explicit mul(symbol_t t = sym_real)
        : function("MUL", t, 2, function::default_weight, true) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(boost::any_cast<double>(ev0) *
                         boost::any_cast<double>(ev1));
        if (isinf(ret))  return boost::any();

        return ret;
      }
    };

    class sin : public function
    {
    public:
      explicit sin(symbol_t t = sym_real) : function("SIN", t, 1) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev(i->eval(0));
        if (ev.empty())  return ev;

        return std::sin(boost::any_cast<double>(ev));
      }
    };

    class sub : public function
    {
    public:
      explicit sub(symbol_t t = sym_real) : function("SUB", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(boost::any_cast<double>(ev0) -
                         boost::any_cast<double>(ev1));
        if (isinf(ret))  return boost::any();

        return ret;
      }
    };
  }  // namespace sr
}  // namespace vita

#endif  // PRIMITIVE_H
