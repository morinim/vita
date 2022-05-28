/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_REAL_PRIMITIVE_H)
#define      VITA_REAL_PRIMITIVE_H

#include <string>

#include "kernel/gp/function.h"
#include "kernel/gp/mep/interpreter.h"
#include "kernel/gp/src/primitive/comp_penalty.h"
#include "kernel/gp/terminal.h"
#include "kernel/random.h"
#include "utility/utility.h"

/// We assume that errors during floating-point operations aren't terminal
/// errors. So we dont't try to prevent domain errors (e.g. square root of a
/// negative number) or range error (e.g. `pow(10.0, 1e6)`) checking arguments
/// beforehand (domain errors could be prevented by carefully bounds checking
/// the arguments before calling functions and taking alternative action if
/// the bounds are violated; range errors usually cannot be prevented, as
/// they are dependent on the implementation of floating-point numbers, as
/// well as the function being applied).
/// Instead we detect them and take alternative action (usually returning
/// an empty value).
namespace vita::real
{

using base_t = D_DOUBLE;

static_assert(std::numeric_limits<base_t>::is_iec559,
              "Vita requires IEC 559/IEEE 754 floating-point types");

///
/// A simple shortcut for casting an `any` to `base_t`.
///
/// \param[in] v the value that must be casted to base type (`base_t`)
/// \return      the content of `v`
///
inline base_t base(const value_t &v)
{
  return std::get<base_t>(v);
}

///
/// Ephemeral random constant.
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
class real : public terminal
{
public:
  explicit real(const cvect &c, base_t m = -1000.0, base_t u = 1000.0)
    : terminal("REAL", c[0]), min(m), upp(u)
  {
    Expects(c.size() == 1);
    Expects(m < u);
  }

  bool parametric() const final { return true; }

  terminal_param_t init() const final { return random::between(min, upp); }

  std::string display(terminal_param_t v, format) const final
  { return std::to_string(v); }

  value_t eval(symbol_params &p) const final
  {
    return static_cast<base_t>(p.fetch_param());
  }

private:
  const base_t min, upp;
};

///
/// Ephemeral random integer constant.
///
/// This is like real::real but restricted to integer numbers.
///
class integer : public terminal
{
public:
  explicit integer(const cvect &c, int m = -128, int u = 127)
    : terminal("REAL", c[0]), min(m), upp(u)
  {
    Expects(c.size() == 1);
    Expects(m < u);
  }

  bool parametric() const final { return true; }

  terminal_param_t init() const final { return random::between(min, upp); }

  std::string display(terminal_param_t v, format) const final
  { return std::to_string(static_cast<int>(v)); }

  value_t eval(symbol_params &p) const final
  {
    return static_cast<base_t>(p.fetch_param());
  }

private:
  const int min, upp;
};

///
/// The absolute value of a real number.
///
class abs : public function
{
public:
  explicit abs(const cvect &c = {0}) : function("FABS", c[0], {c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::abs(%%1%%)";
    case mql_format:     return  "MathAbs(%%1%%)";
    case python_format:  return      "abs(%%1%%)";
    default:             return     "fabs(%%1%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a(args[0]);
    return has_value(a) ? std::fabs(base(a)) : a;
  }
};

///
/// Sum of two real numbers.
///
class add : public function
{
public:
  explicit add(const cvect &c = {0}) : function("FADD", c[0], {c[0], c[0]}) {}

  bool associative() const final { return true; }

  std::string display(format) const final
  {
    return "(%%1%%+%%2%%)";
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(base(a0) + base(a1));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// Analytic quotient (AQ).
///
/// Analytic quotient (AQ) operator systematically yields lower mean squared
/// errors over a range of regression tasks, due principally to removing the
/// discontinuities or singularities that can often result from using either
/// protected or unprotected division. Further, the AQ operator is
/// differentiable.
///
class aq : public function
{
public:
  explicit aq(const cvect &c = {0}) : function("AQ", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "(%%1%%/std::sqrt(1.0+std::pow(%%2%%,2.0)))";
    case mql_format:     return       "(%%1%%/MathSqrt(1+MathPow(%%2%%,2)))";
    default:             return           "(%%1%%/sqrt(1.0+pow(%%2%%,2.0)))";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const auto x(base(a0)), y(base(a1));
    const base_t ret(x / std::sqrt(1.0 + y * y));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// `cos()` of a real number.
///
class cos : public function
{
public:
  explicit cos(const cvect &c = {0}) : function("FCOS", c[0], {c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::cos(%%1%%)";
    case mql_format:     return  "MathCos(%%1%%)";
    default:             return      "cos(%%1%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a(args[0]);
    if (!has_value(a))  return a;

    return std::cos(base(a));
  }
};

///
/// Unprotected division (UPD) between two real numbers.
///
class div : public function
{
public:
  explicit div(const cvect &c = {0}) : function("FDIV", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format) const final
  {
    return "(%%1%%/%%2%%)";
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(base(a0) / base(a1));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// "Greater Than" operator.
///
class gt : public function
{
public:
  explicit gt(const cvect &c = {0, 0}) : function(">", c[1], {c[0], c[0]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:  return "std::isgreater(%%1%%,%%2%%)";
    default:          return "(%%1%%>%%2%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    return std::isgreater(base(a0), base(a1));
    // If one or both arguments of isgreater are NaN, the function returns
    // `false`, but no FE_INVALID exception is raised (note that the
    // expression `v0 < v1` may raise an exception in this case).
  }
};

///
/// Quotient of the division between two real numbers.
///
class idiv : public function
{
public:
  explicit idiv(const cvect &c = {0}) : function("FIDIV", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::floor(%%1%%/%%2%%)";
    case mql_format:     return  "MathFloor(%%1%%/%%2%%)";
    case python_format:  return          "(%%1%%//%%2%%)";
    default:             return      "floor(%%1%%/%%2%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(std::floor(base(a0) / base(a1)));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// "If between" operator.
///
/// \warning Requires five input arguments.
///
class ifb : public function
{
public:
  explicit ifb(const cvect &c = {0, 0})
    : function("FIFB", c[1], {c[0], c[0], c[0], c[1], c[1]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case python_format:
      return "(%%4%% if %%2%% <= %%1%% <= %%3%% else %%5%%)";
    default:
      return "(fmin(%%2%%,%%3%%) <= %%1%% && %%1%% <= fmax(%%2%%,%%3%%) ?"
             "%%4%% : %%5%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const auto a2(args[2]);
    if (!has_value(a2))  return a2;

    const auto v0(base(a0));
    const auto v1(base(a1));
    const auto v2(base(a2));

    const auto min(std::fmin(v1, v2));
    const auto max(std::fmax(v1, v2));

    if (std::isless(v0, min) || std::isgreater(v0, max))
      return args[4];
    else
      return args[3];
  }
};

///
/// "If equal" operator.
///
class ife : public function
{
public:
  explicit ife(const cvect &c = {0, 0})
    : function("FIFE", c[1], {c[0], c[0], c[1], c[1]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:
      return "(abs(%%1%%-%%2%%)<2*std::numeric_limits<T>::epsilon() ?"
             "%%3%% : %%4%%)";
    case mql_format:
      return "(NormalizeDouble(%%1%%-%%2%%,8)==0 ? %%3%% : %%4%%)";
    case python_format:
      return "(%%3%% if isclose(%%1%%, %%2%%) else %%4%%)";
    default:
      return "(fabs(%%1%%-%%2%%) < 2*DBL_EPSILON ? %%3%% : %%4%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    if (issmall(base(a0) - base(a1)))
      return args[2];
    else
      return args[3];
  }

  double penalty_nvi(core_interpreter *ci) const final
  {
    return comparison_function_penalty(ci);
  }
};

///
/// "If less then" operator.
///
class ifl : public function
{
public:
  explicit ifl(const cvect &c  = {0, 0})
    : function("FIFL", c[1], {c[0], c[0], c[1], c[1]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case python_format:  return "(%%3%% if %%1%%<%%2%% else %%4%%)";
    default:             return     "(%%1%%<%%2%% ? %%3%% : %%4%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const auto v0(base(a0)), v1(base(a1));
    if (std::isless(v0, v1))
      return args[2];
    else
      return args[3];
  }

  double penalty_nvi(core_interpreter *ci) const final
  {
    return comparison_function_penalty(ci);
  }
};

///
/// "If zero" operator.
///
class ifz : public function
{
public:
  explicit ifz(const cvect &c = {0})
    : function("FIFZ", c[0], {c[0], c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:
      return "(abs(%%1%%)<2*std::numeric_limits<T>::epsilon() ?"
             "%%3%% : %%4%%)";
    case mql_format:
      return "(NormalizeDouble(%%1%%,8)==0 ? %%3%% : %%4%%)";
    case python_format:
      return "(%%3%% if abs(%%1%%) < 1e-10 else %%4%%)";
    default:
      return "(fabs(%%1%%)<2*DBL_EPSILON ? %%3%% : %%4%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    if (issmall(base(a0)))
      return args[1];
    else
      return args[2];
  }
};

///
/// Length of a string.
///
class length : public function
{
public:
  explicit length(const cvect &c = {0, 0}) : function("FLENGTH", c[1], {c[0]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::string(%%1%%).length()";
    case mql_format:     return            "StringLen(%%1%%)";
    case python_format:  return                  "len(%%1%%)";
    default:             return               "strlen(%%1%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a(args[0]);
    if (!has_value(a))  return a;

    return static_cast<base_t>(std::get<D_STRING>(a).length());
  }
};

///
/// Natural logarithm of a real number.
///
class ln : public function
{
public:
  explicit ln(const cvect &c = {0}) : function("FLN", c[0], {c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:  return "std::log(%%1%%)";
    case mql_format:  return  "MathLog(%%1%%)";
    default:          return      "log(%%1%%)";
    }
  }

  ///
  /// \param[in] i pointer to the active interpreter
  /// \return      the natural logarithm of its argument or an empty value in
  ///              case of invalid argument / infinite result
  ///
  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto ret(std::log(base(a0)));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// "Less Then" operator.
///
class lt : public function
{
public:
  explicit lt(const cvect &c = {0, 0}) : function("<", c[1], {c[0], c[0]})
  { Expects(c.size() == 2); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:  return "std::isless(%%1%%,%%2%%)";
    default:          return            "(%%1%%<%%2%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    return std::isless(base(a0), base(a1));
    // If one or both arguments of `isless` are NaN, the function returns
    // false, but no FE_INVALID exception is raised (note that the
    // expression `v0 < v1` may raise an exception in this case).
  }
};

///
/// The larger of two floating point values.
///
class max : public function
{
public:
  explicit max(const cvect &c = {0}) : function("FMAX", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case python_format:  return  "max(%%1%%,%%2%%)";
    default:             return "fmax(%%1%%,%%2%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(std::fmax(base(a0), base(a1)));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// Remainder of the division between real numbers.
///
class mod : public function
{
public:
  explicit mod(const cvect &c = {0}) : function("FMOD", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::fmod(%%1%%,%%2%%)";
    case mql_format:     return   "MathMod(%%1%%,%%2%%)";
    case python_format:  return        "(%%1%% % %%2%%)";
    default:             return      "fmod(%%1%%,%%2%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(std::fmod(base(a0), base(a1)));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// Product of real numbers.
///
class mul : public function
{
public:
  explicit mul(const cvect &c = {0}) : function("FMUL", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format) const final
  {
    return "(%%1%%*%%2%%)";
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(base(a0) * base(a1));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};

///
/// sin() of a real number.
///
class sin : public function
{
public:
  explicit sin(const cvect &c = {0}) : function("FSIN", c[0], {c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::sin(%%1%%)";
    case mql_format:     return  "MathSin(%%1%%)";
    default:             return      "sin(%%1%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a(args[0]);
    if (!has_value(a))  return a;

    return std::sin(base(a));
  }
};

///
/// Square root of a real number.
///
class sqrt : public function
{
public:
  explicit sqrt(const cvect &c = {0}) : function("FSQRT", c[0], {c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "std::sqrt(%%1%%)";
    case mql_format:     return  "MathSqrt(%%1%%)";
    default:             return      "sqrt(%%1%%)";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a(args[0]);
    if (!has_value(a))  return a;

    const auto v(base(a));
    if (std::isless(v, 0.0))
      return {};

    return std::sqrt(v);
  }
};

///
/// Subtraction between real numbers.
///
class sub : public function
{
public:
  explicit sub(const cvect &c = {0}) : function("FSUB", c[0], {c[0], c[0]})
  { Expects(c.size() == 1); }

  std::string display(format) const final
  {
    return "(%%1%%-%%2%%)";
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    const auto a1(args[1]);
    if (!has_value(a1))  return a1;

    const base_t ret(base(a0) - base(a1));
    if (!std::isfinite(ret))  return {};

    return ret;
  }
};


///
/// Sigmoid function.
///
class sigmoid : public function
{
public:
  explicit sigmoid(const cvect &c = {0}) : function("FSIGMOID", c[0], {c[0]})
  { Expects(c.size() == 1); }

  std::string display(format f) const final
  {
    switch (f)
    {
    case cpp_format:     return "1.0 / (1.0 + std::exp(-%%1%%))";
    case mql_format:     return  "1.0 / (1.0 + MathExp(-%%1%%))";
    case python_format:  return   "1. / (1. + math.exp(-%%1%%))";
    default:             return          "1 / (1 + exp(-%%1%%))";
    }
  }

  value_t eval(symbol_params &args) const final
  {
    const auto a0(args[0]);
    if (!has_value(a0))  return a0;

    // The sigmoid function can be expressed in one of two equivalent ways:
    //     sigmoid(x) = 1 / (1 + exp(-x)) = exp(x) / (exp(x) + 1)
    // Each version can be used in order to avoid numerical overflow in extreme
    // cases (`x --> +inf` and `x --> -inf` respectively).
    const auto x(base(a0));
    if (x >= 0.0)
      return 1.0 / (1.0 + std::exp(-x));

    return std::exp(x) / (1.0 + std::exp(x));
  }
};

}  // namespace vita::real

#endif  // include guard
