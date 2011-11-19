/**
 *
 *  \file double.h
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

#if !defined(DOUBLE_PRIMITIVE_H)
#define      DOUBLE_PRIMITIVE_H

#include <boost/any.hpp>

#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita
{
  /// We assume that errors during floating-point operations aren't terminal
  /// error. So we dont't try to prevent domain errors (e.g. square root of a
  /// negative number) or range error (e.g. pow(10.0, 1e6)) checking arguments
  /// beforehand (domain errors could be prevented by carefully bounds checking
  /// the arguments before calling functions and taking alternative action if
  /// the bounds are violated; range errors usually can not be prevented, as
  /// they are dependent on the implementation of floating-point numbers, as
  /// well as the function being applied).
  /// Instead we detect them and take alternative action (usually returning
  /// an empty boost::any()).
  namespace dbl
  {
    ///
    /// It is assumed that the creation of floating-point constants is
    /// necessary to do symbolic regression in evolutionary computation.
    /// Genetic programming solves the problem of constant creation by using a
    /// special terminal named "ephemeral random constant" (Koza 1992). For
    /// each ephemeral random constant used in the initial population, a random
    /// number of a special data type in a specified range is generated. Then
    /// these random constants are moved around from tree to tree by the
    /// crossover operator.
    ///
    class number : public terminal
    {
    public:
      explicit number(category_t t = 0, int m = -128, int u = 127)
        : terminal("NUM", t, false, true, default_weight*2), min(m), upp(u) {}

      int init() const { return random::between<int>(min, upp); }

      std::string display(int v) const
      { return boost::lexical_cast<std::string>(v); }

      boost::any eval(interpreter *i) const
      { return static_cast<double>(boost::any_cast<int>(i->eval())); }

    private:
      const int min, upp;
    };

    class abs : public function
    {
    public:
      explicit abs(category_t t = 0) : function("ABS", t, 1) {}

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
      explicit add(category_t t = 0)
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
      explicit div(category_t t = 0) : function("DIV", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(boost::any_cast<double>(ev0) /
                         boost::any_cast<double>(ev1));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }
    };

    class idiv : public function
    {
    public:
      explicit idiv(category_t t = 0) : function("IDIV", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(std::floor(boost::any_cast<double>(ev0) /
                                    boost::any_cast<double>(ev1)));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }
    };

    class ife : public function
    {
    public:
      explicit ife(category_t t1 = 0, category_t t2 = 0)
        : function("IFE", t1, {t2, t2, t1, t1}) {}

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
      explicit ifl(category_t t1 = 0, category_t t2 = 0)
        : function("IFL", t1, {t2, t2, t1, t1}) {}

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
      explicit ifz(category_t t = 0) : function("IFZ", t, 3) {}

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
      explicit ln(category_t t = 0)
        : function("LN", t, 1, function::default_weight/2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const double ret(std::log(boost::any_cast<double>(i->eval(0))));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }
    };

    class mod : public function
    {
    public:
      explicit mod(category_t t = 0) : function("MOD", t, 2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const double ret(std::fmod(boost::any_cast<double>(ev0),
                                   boost::any_cast<double>(ev1)));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }
    };

    class mul : public function
    {
    public:
      explicit mul(category_t t = 0)
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
      explicit sin(category_t t = 0) : function("SIN", t, 1) {}

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
      explicit sub(category_t t = 0) : function("SUB", t, 2) {}

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
  }  // namespace dbl
}  // namespace vita

#endif  // DOUBLE_PRIMITIVE_H
