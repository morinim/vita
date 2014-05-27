/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DOUBLE_PRIMITIVE_H)
#define      VITA_DOUBLE_PRIMITIVE_H

#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"
#include "kernel/utility.h"

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
    /// \brief Ephemeral random constant
    ///
    /// It is assumed that the creation of floating-point constants is
    /// necessary to do symbolic regression in evolutionary computation.
    /// Genetic programming solves the problem of constant creation by using a
    /// special terminal named "ephemeral random constant" (Koza 1992). For
    /// each ephemeral random constant used in the initial population, a random
    /// number of a special data type in a specified range is generated. Then
    /// these random constants are moved around from genome to genome by the
    /// crossover operator.
    ///
    class number : public terminal
    {
    public:
      explicit number(category_t t, int m = -128, int u = 127)
        : terminal("REAL", t, false, true, k_base_weight), min(m), upp(u)
      { assert(m < u); }

      virtual int init() const override
      { return random::between<int>(min, upp); }

      virtual std::string display(int v) const override
      { return std::to_string(v); }

      virtual any eval(interpreter<i_mep> *i) const override
      { return any(static_cast<base_t>(any_cast<int>(i->fetch_param()))); }

    private: // Private data members.
      const int min, upp;
    };

    ///
    /// \brief The absolute value of a real number
    ///
    class abs : public function
    {
    public:
      explicit abs(category_t t) : function("FABS", t, {t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a(i->fetch_arg(0));

        return a.empty() ? a : any(std::fabs(dbl::cast(a)));
      }
    };

    ///
    /// \brief Sum of two real numbers
    ///
    class add : public function
    {
    public:
      explicit add(category_t t)
        : function("FADD", t, {t, t}, k_base_weight, true) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(dbl::cast(a0) + dbl::cast(a1));
        if (std::isinf(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// Division between two real numbers
    ///
    class div : public function
    {
    public:
      explicit div(category_t t) : function("FDIV", t, {t, t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(dbl::cast(a0) / dbl::cast(a1));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// \brief Quotient of the division between two real numbers
    ///
    class idiv : public function
    {
    public:
      explicit idiv(category_t t) : function("FIDIV", t, {t, t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(std::floor(dbl::cast(a0) / dbl::cast(a1)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// \brief "If between" operator
    ///
    /// \note Requires five input arguments.
    ///
    class ifb : public function
    {
    public:
      ifb(category_t t1, category_t t2)
        : function("FIFB", t2, {t1, t1, t1, t2, t2})
      { assert(gene::k_args > 4); }

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const any a2(i->fetch_arg(2));
        if (a2.empty())  return a2;

        const auto v0(dbl::cast(a0));
        const auto v1(dbl::cast(a1));
        const auto v2(dbl::cast(a2));

        const auto min(std::fmin(v1, v2));
        const auto max(std::fmax(v1, v2));

        if (min <= v0 && v0 <= max)
          return i->fetch_arg(3);
        else
          return i->fetch_arg(4);
      }
    };

    ///
    /// \brief "If equal" operator
    ///
    class ife : public function
    {
    public:
      ife(category_t t1, category_t t2)
        : function("FIFE", t2, {t1, t1, t2, t2}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        if (issmall(dbl::cast(a0) - dbl::cast(a1)))
          return i->fetch_arg(2);
        else
          return i->fetch_arg(3);
      }
    };

    ///
    /// \brief "If less then" operator
    ///
    class ifl : public function
    {
    public:
      ifl(category_t t1, category_t t2)
        : function("FIFL", t2, {t1, t1, t2, t2}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const auto v0(dbl::cast(a0)), v1(dbl::cast(a1));
        if (std::isless(v0, v1))
          return i->fetch_arg(2);
        else
          return i->fetch_arg(3);

        // If one or both arguments of isless are Nan, the function returns
        // false, but no FE_INVALID exception is raised (note that the
        // expression v0 < v1 may rais an exception in this case).
      }
    };

    ///
    /// \brief "If zero" operator
    ///
    class ifz : public function
    {
    public:
      explicit ifz(category_t t) : function("FIFZ", t, {t, t, t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        if (issmall(dbl::cast(a0)))
          return i->fetch_arg(1);
        else
          return i->fetch_arg(2);
      }
    };

    ///
    /// \brief Length of a string
    ///
    class length : public function
    {
    public:
      explicit length(category_t t1, category_t t2)
        : function("FLENGTH", t2, {t1}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a(i->fetch_arg(0));
        if (a.empty())  return a;

        return any(static_cast<base_t>(any_cast<std::string>(a).size()));
      }
    };

    ///
    /// \brief Natural logarithm of a real number.
    ///
    class ln : public function
    {
    public:
      explicit ln(category_t t) : function("FLN", t, {t}, k_base_weight / 2) {}

      ///
      /// \param[in] i pointer to the active interpreter.
      /// \return the natural logarithm of its argument or an empty \c any
      //          in case of invalid argument / infinite result.
      ///
      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const base_t ret(std::log(dbl::cast(a0)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// \brief The larger of two floating point values
    ///
    class max : public function
    {
    public:
      explicit max(category_t t) : function("FMAX", t, {t, t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(std::fmax(dbl::cast(a0), dbl::cast(a1)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// \brief Remainder of the division between real numbers
    ///
    class mod : public function
    {
    public:
      explicit mod(category_t t) : function("FMOD", t, {t, t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(std::fmod(dbl::cast(a0), dbl::cast(a1)));
        if (!std::isfinite(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// \brief Product of real numbers
    ///
    class mul : public function
    {
    public:
      explicit mul(category_t t)
        : function("FMUL", t, {t, t}, k_base_weight, true) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(dbl::cast(a0) * dbl::cast(a1));
        if (std::isinf(ret))  return any();

        return any(ret);
      }
    };

    ///
    /// \brief sin() of a real number
    ///
    class sin : public function
    {
    public:
      explicit sin(category_t t) : function("FSIN", t, {t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a(i->fetch_arg(0));
        if (a.empty())  return a;

        return any(std::sin(dbl::cast(a)));
      }
    };

    ///
    /// \brief square root of a real number
    ///
    class sqrt : public function
    {
    public:
      explicit sqrt(category_t t) : function("FSQRT", t, {t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a(i->fetch_arg(0));
        if (a.empty())  return a;

        const auto v(dbl::cast(a));
        if (std::isless(v, 0.0))
          return any();

        return any(std::sqrt(v));
      }
    };

    ///
    /// \brief Subtraction between real numbers
    ///
    class sub : public function
    {
    public:
      explicit sub(category_t t) : function("FSUB", t, {t, t}) {}

      virtual any eval(interpreter<i_mep> *i) const override
      {
        const any a0(i->fetch_arg(0));
        if (a0.empty())  return a0;

        const any a1(i->fetch_arg(1));
        if (a1.empty())  return a1;

        const base_t ret(dbl::cast(a0) - dbl::cast(a1));
        if (std::isinf(ret))  return any();

        return any(ret);
      }
    };
  }  // namespace dbl
}  // namespace vita

#endif  // Include guard
