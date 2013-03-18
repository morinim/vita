/**
 *
 *  \file double.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(DOUBLE_PRIMITIVE_H)
#define      DOUBLE_PRIMITIVE_H

#include <string>

#include "function.h"
#include "interpreter.h"
#include "random.h"
#include "terminal.h"

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
  /// an empty \c any()).
  namespace dbl
  {
    typedef double base_t;

    ///
    /// \param[in] v the value that must be casted to base type (\c base_t).
    ///
    /// Just a simple shortcut.
    inline
    base_t cast(const any &v) { return any_cast<base_t>(v); }

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
      explicit number(category_t t, int m = -128, int u = 127)
        : terminal("REAL", t, false, true, default_weight), min(m), upp(u)
      { assert(m < u); }

      int init() const { return random::between<int>(min, upp); }

      std::string display(int v) const
      { return boost::lexical_cast<std::string>(v); }

      virtual any eval(interpreter *i) const
      { return any(static_cast<base_t>(any_cast<int>(i->eval()))); }

    private: // Private data members.
      const int min, upp;
    };

    class abs : public function
    {
    public:
      explicit abs(category_t t) : function("FABS", t, {t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev(i->eval(0));
        if (ev.empty())  return ev;

        return any(std::fabs(dbl::cast(ev)));
      }
    };

    class add : public function
    {
    public:
      explicit add(category_t t)
        : function("FADD", t, {t, t}, function::default_weight, true) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) + dbl::cast(ev1));
        if (std::isinf(ret))  return any();

        return any(ret);
      }
    };

    class div : public function
    {
    public:
      explicit div(category_t t) : function("FDIV", t, {t, t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) / dbl::cast(ev1));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    class idiv : public function
    {
    public:
      explicit idiv(category_t t) : function("FIDIV", t, {t, t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(std::floor(dbl::cast(ev0) / dbl::cast(ev1)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    class ifb : public function
    {
    public:
      ifb(category_t t1, category_t t2)
        : function("FIFB", t2, {t1, t1, t1, t2, t2})
      { assert(gene::k_args > 4); }

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const any ev2(i->eval(2));
        if (ev2.empty())  return ev2;

        const base_t v0(dbl::cast(ev0));
        const base_t v1(dbl::cast(ev1));
        const base_t v2(dbl::cast(ev2));

        const base_t min(std::fmin(v1, v2));
        const base_t max(std::fmax(v1, v2));

        if (min <= v0 && v0 <= max)
          return i->eval(3);
        else
          return i->eval(4);
      }
    };

    class ife : public function
    {
    public:
      ife(category_t t1, category_t t2)
        : function("FIFE", t2, {t1, t1, t2, t2}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t cmp(std::fabs(dbl::cast(ev0) - dbl::cast(ev1)));

        if (cmp < float_epsilon)
          return i->eval(2);
        else
          return i->eval(3);
      }
    };

    class ifl : public function
    {
    public:
      ifl(category_t t1, category_t t2)
        : function("FIFL", t2, {t1, t1, t2, t2}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        if (dbl::cast(ev0) < dbl::cast(ev1))
          return i->eval(2);
        else
          return i->eval(3);
      }
    };

    class ifz : public function
    {
    public:
      explicit ifz(category_t t) : function("FIFZ", t, {t, t, t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        if (std::fabs(dbl::cast(ev0)) < float_epsilon)
          return i->eval(1);
        else
          return i->eval(2);
      }
    };

    class length : public function
    {
    public:
      explicit length(category_t t1, category_t t2)
        : function("FLENGTH", t2, {t1}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev(i->eval(0));
        if (ev.empty())  return ev;

        return any(static_cast<base_t>(any_cast<std::string>(ev).size()));
      }
    };

    ///
    /// Simply the natural logarithm of a real number.
    ///
    class ln : public function
    {
    public:
      explicit ln(category_t t)
        : function("FLN", t, {t}, function::default_weight / 2) {}

      ///
      /// \param[in] i pointer to the active interpreter.
      /// \return the natural logarithm of its argument or an empty \c any
      //          in case of invalid argument / infinite result.
      ///
      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const base_t ret(std::log(dbl::cast(ev0)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    class mod : public function
    {
    public:
      explicit mod(category_t t) : function("FMOD", t, {t, t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(std::fmod(dbl::cast(ev0), dbl::cast(ev1)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    class mul : public function
    {
    public:
      explicit mul(category_t t)
        : function("FMUL", t, {t, t}, function::default_weight, true) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) * dbl::cast(ev1));
        if (std::isinf(ret))  return any();

        return any(ret);
      }
    };

    class sin : public function
    {
    public:
      explicit sin(category_t t) : function("FSIN", t, {t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev(i->eval(0));
        if (ev.empty())  return ev;

        return any(std::sin(dbl::cast(ev)));
      }
    };

    class sub : public function
    {
    public:
      explicit sub(category_t t) : function("FSUB", t, {t, t}) {}

      virtual any eval(interpreter *i) const
      {
        const any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) - dbl::cast(ev1));
        if (std::isinf(ret))  return any();

        return any(ret);
      }
    };
  }  // namespace dbl
}  // namespace vita

#endif  // DOUBLE_PRIMITIVE_H
