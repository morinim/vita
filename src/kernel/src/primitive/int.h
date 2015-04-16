/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INT_PRIMITIVE_H)
#define      VITA_INT_PRIMITIVE_H

#include <climits>
#include <limits>
#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"
#include "kernel/src/primitive/comp_penalty.h"

namespace vita
{
/// Integer overflow is undefined behaviour. This means that implementations
/// have a great deal of latitude in how they deal with signed integer
/// overflow. An implementation that defines signed integer types as being
/// modulo, for example, need not detect integer overflow. Implementations
/// may also trap on signed arithmetic overflows, or simply assume that
/// overflows will never happen and generate object code accordingly. For
/// these reasons, it is important to ensure that operations on signed
/// integers do no result in signed overflow
namespace integer
{
using base_t = int;

///
/// \param[in] v the value that must be casted to base type (`base_t`).
///
/// Just a simple shortcut.
inline base_t cast(const any &v) { return any_cast<base_t>(v); }

///
/// Integer ephemeral random constant.
/// \see dbl::number
///
class number : public terminal
{
public:
  explicit number(const cvect &c, int m = -128, int u = 127)
    : terminal("INT", c[0]), min(m), upp(u)
  {
    assert(c.size() == 1);
    assert(m < u);
  }

  virtual bool parametric() const override { return true; }

  virtual double init() const override
  { return random::between<int>(min, upp); }

  virtual std::string display(double v) const override
  { return std::to_string(v); }

  virtual any eval(core_interpreter *i) const override
  {
    return any(static_cast<base_t>(
                 any_cast<gene::param_type>(
                   static_cast<interpreter<i_mep> *>(i)->fetch_param())));
  }

private:  // Private data members
  const int min, upp;
};

/// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Addition
class add : public function
{
public:
  explicit add(const cvect &c) : function("ADD", c[0], {c[0], c[0]})
  {
    assert(c.size() == 1);
  }

  virtual bool associative() const override { return true; }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v0 > 0 && v1 > 0 && (v0 > std::numeric_limits<base_t>::max() - v1))
      return any(std::numeric_limits<base_t>::max());
    if (v0 < 0 && v1 < 0 && (v0 < std::numeric_limits<base_t>::min() - v1))
      return any(std::numeric_limits<base_t>::min());

    return any(v0 + v1);
  }
};

/// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Division
class div : public function
{
public:
  explicit div(const cvect &c) : function("DIV", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v1 == 0 || (v0 == std::numeric_limits<base_t>::min() && (v1 == -1)))
      return any(v0);

    return any(v0 / v1);
  }
};


class ife : public function
{
public:
  explicit ife(const cvect &c)
    : function("IFE", c[1], {c[0], c[0], c[1], c[1]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v0 == v1)
      return i->fetch_arg(2);

    return i->fetch_arg(3);
  }

  virtual double penalty_nvi(core_interpreter *ci) const override
  {
    return comparison_function_penalty(ci);
  }
};

class ifl : public function
{
public:
  explicit ifl(const cvect &c)
    : function("IFL", c[1], {c[0], c[0], c[1], c[1]})
  { assert(c.size() == 2); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v0 < v1)
      return i->fetch_arg(2);

    return i->fetch_arg(3);
  }

  virtual double penalty_nvi(core_interpreter *ci) const override
  {
    return comparison_function_penalty(ci);
  }
};

class ifz : public function
{
public:
  explicit ifz(const cvect &c) : function("IFZ", c[0], {c[0], c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));

    if (v0 == 0)
      return i->fetch_arg(1);

    return i->fetch_arg(2);
  }

  virtual double penalty_nvi(core_interpreter *ci) const override
  {
    return comparison_function_penalty(ci);
  }
};

/// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Modulo
class mod : public function
{
public:
  explicit mod(const cvect &c) : function("MOD", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v1 == 0 || (v0 == std::numeric_limits<base_t>::min() && (v1 == -1)))
      return any(v1);

    return any(v0 % v1);
  }
};

/// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Multiplication
class mul : public function
{
public:
  explicit mul(const cvect &c) : function("MUL", c[0], {c[0], c[0]}) {}

  virtual bool associative() const override { return true; }

  virtual any eval(core_interpreter *ci) const
  {
    static_assert(sizeof(long long) >= 2 * sizeof(base_t),
                  "Unable to detect overflow after multiplication");

    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    long long tmp(v0 * v1);
    if (tmp > std::numeric_limits<base_t>::max())
      return any(std::numeric_limits<base_t>::max());
    if (tmp < std::numeric_limits<base_t>::min())
      return any(std::numeric_limits<base_t>::min());

    return any(static_cast<base_t>(tmp));

    /*
    // On systems where the above relationship does not hold, the following
    // compliant solution may be used to ensure signed overflow does not
    // occur.
    if (v0 > 0)
      if (v1 > 0)
      {
        assert(v0 > 0 && v1 > 0);
        if (v0 > std::numeric_limits<base_t>::max() / v1)
        return any(std::numeric_limits<base_t>::max());
      }
      else  // v0 is positive, v1 is non-positive
      {
        assert(v0 > 0 && v1 <= 0);
        if (v1 < std::numeric_limits<base_t>::min() / v0)
          return any(std::numeric_limits<base_t>::min());
      }
    else  // v0 is non-positive
      if (v1 > 0)
      {
        assert(v0 <= 0 && v1 > 0);
        if (v0 < std::numeric_limits<base_t>::min() / v1)
          return any(std::numeric_limits<base_t>::min());
      }
      else  // v0 is non-positive, v1 is non-positive
      {
        assert(v0 <= 0 && v1 <= 0);
        if (v0 != 0 && v1 < std::numeric_limits<base_t>::max() / v0)
          return any(std::numeric_limits<base_t>::max());
      }

    return any(v0 * v1);
    */
  }
};

/// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-LeftShiftOperator
class shl : public function
{
public:
  explicit shl(const cvect &c) : function("SHL", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v0 < 0 || v1 < 0 ||
        v1 >= static_cast<base_t>(sizeof(base_t) * CHAR_BIT) ||
        v0 > std::numeric_limits<base_t>::max() >> v1)
      return any(v0);

    return any(v0 << v1);
  }
};

/// \see https://www.securecoding.cert.org/confluence/display/cplusplus/INT32-CPP.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow#INT32-CPP.Ensurethatoperationsonsignedintegersdonotresultinoverflow-Subtraction
class sub : public function
{
public:
  explicit sub(const cvect &c) : function("SUB", c[0], {c[0], c[0]})
  { assert(c.size() == 1); }

  virtual any eval(core_interpreter *ci) const override
  {
    auto *const i(static_cast<interpreter<i_mep> *>(ci));
    const auto v0(integer::cast(i->fetch_arg(0)));
    const auto v1(integer::cast(i->fetch_arg(1)));

    if (v0 < 0 && v1 > 0 && (v0 < std::numeric_limits<base_t>::min() + v1))
      return any(std::numeric_limits<base_t>::min());
    if (v0 > 0 && v1 < 0 && (v0 > std::numeric_limits<base_t>::max() + v1))
      return any(std::numeric_limits<base_t>::max());

    return any(v0 - v1);
  }
};

}  // namespace integer
}  // namespace vita

#endif  // Include guard
