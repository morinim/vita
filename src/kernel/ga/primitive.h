/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2022 EOS di Manlio Morini.
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
#include "kernel/gp/terminal.h"

namespace vita::ga
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
/// an empty value).
using base_t = terminal_param_t;

namespace detail
{
template<class T>
class number : public terminal
{
public:
  using value_type = T;

  /// A number (terminal symbol) within a range used for genetics algorithms.
  ///
  /// \param[in] name the name of the terminal
  /// \param[in] r    a half-open range
  /// \param[in] i    a locus of the chromosome (default value means
  ///                 unspecified and is used for a standard, uniform GA
  ///                 chromosome)
  ///
  /// This is a base helper class used to build more specific numeric classes.
  /// The general idea follows:
  /// - **the problem can be tackled with a standard, uniform chromosome**
  ///   (every locus contain the same kind of gene). In this case the user
  ///   simply calls the `ga_problem`/`de_problem` constructor specifying the
  ///   length of the chromosome;
  /// - **the problem requires a more complex structure**. The user specifies a
  ///   (possibly) different type for every locus.
  ///
  explicit number(const std::string &name, range_t<T> r, category_t i)
    : terminal(name, i), range_(r)
  {
    Expects(r.first < r.second);
  }

  bool parametric() const final { return true; }

  terminal_param_t init() const override { return random::in(range_); }

  std::string display(terminal_param_t v, format) const override
  { return std::to_string(static_cast<T>(v)); }

private:
  /// \warning i_ga / i_de directly access the genome vector.
  value_t eval(symbol_params &) const override { return {}; }

  const range_t<T> range_;
};

}  // namespace detail

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
class real : public detail::number<double>
{
public:
  // \param[in] r a half open range
  // \param[in] i an optional category
  explicit real(range_t<double> r = {-1000.0, 1000.0},
                category_t i = undefined_category)
    : detail::number<double>("REAL", r, i)
  {
  }
};

class integer : public detail::number<int>
{
public:
  // \param[in] r a half open range
  // \param[in] i an optional category
  explicit integer(range_t<int> r = {-1000, 1000},
                   category_t i = undefined_category)
    : detail::number<int>("INTEGER", r, i)
  {
  }
};

}  // namespace vita::ga

#endif  // include guard
