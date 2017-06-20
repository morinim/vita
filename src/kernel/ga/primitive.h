/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_PRIMITIVE_H)
#define      VITA_GA_PRIMITIVE_H

#include <string>

#include "kernel/random.h"
#include "kernel/terminal.h"
#include "kernel/ga/interpreter.h"

namespace vita
{
/// We assume that errors during floating-point operations aren't terminal
/// error. So we dont't try to prevent domain errors (e.g. square root of a
/// negative number) or range error (e.g. `pow(10.0, 1e6)`) checking
/// arguments beforehand (domain errors could be prevented by carefully
/// bounds checking the arguments before calling functions and taking
/// alternative action if the bounds are violated; range errors usually can
/// not be prevented, as they are dependent on the implementation of
/// floating-point numbers, as well as the function being applied).
/// Instead we detect them and take alternative action (usually returning
/// an empty `any()`).
namespace ga
{
using base_t = terminal::param_t;

template<class T>
class number : public terminal
{
public:
  using value_t = T;

  explicit number(const std::string &name, category_t c,
                  T m = T(-1000), T u = T(1000))
    : terminal(name, c), min(m), upp(u)
  {
    Expects(m < u);
  }

  bool parametric() const final { return true; }

  terminal::param_t init() const override
  { return random::between<T>(min, upp); }

  std::string display(terminal::param_t v, format) const override
  { return std::to_string(static_cast<T>(v)); }

  /// \warning
  /// This works but isn't very useful. i_ga / ga_evaluator classes directly
  /// access the value of a real object.
  any eval(core_interpreter *i) const override
  {
    return any(
      static_cast<interpreter<i_ga> *>(i)->fetch_param(category()));
  }

  double penalty_nvi(core_interpreter *i) const override
  {
    const auto v(static_cast<interpreter<i_ga> *>(i)->fetch_param(category()));

    if (std::isnan(v))
      return std::numeric_limits<double>::max();

    if (v < min)
      return min - v;

    if (v >= upp)
      return v - upp;

    return 0.0;
  }

private:
  const T min, upp;
};

///
/// Mainly used for differential evolution.
///
/// While many genetic algorithms use integers to approximate continuous
/// parameters, the choice limits the resolution with which an optimum can
/// be located. Floating point not only uses computer resources
/// efficiently, it also makes input and output transparent for the user.
/// Parameters can be input, manipulated and output as ordinary
/// floating-point numbers without ever being reformatted as genes with a
/// different binary representation.
///
class real : public number<double>
{
public:
  explicit real(category_t c, double m = -1000.0, double u = 1000.0)
    : number<double>("REAL", c, m, u)
  {
  }
};

class integer : public number<int>
{
public:
  explicit integer(category_t c, int m = -1000, int u = 1000)
    : number<int>("INTEGER", c, m, u)
  {
  }
};

///
/// A convenient shortcut to build the symbol set of a GA problem.
///
/// \param[in] i this will be the i-th arguments
/// \param[in] m minimum value of the argument
/// \param[in] u upper limit for the argument
/// \return      a pointer to the created parameter
///
template<class T = real>
std::unique_ptr<symbol> parameter(unsigned i, base_t m = -1000.0,
                                  base_t u = 1000.0)
{
  return std::make_unique<T>(static_cast<category_t>(i),
                             static_cast<typename T::value_t>(m),
                             static_cast<typename T::value_t>(u));
}

}  // namespace ga
}  // namespace vita

#endif  // include guard
