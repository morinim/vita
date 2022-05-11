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

#if !defined(VITA_FUNCTION_H)
#define      VITA_FUNCTION_H

#include <string>

#include "kernel/gp/symbol.h"

namespace vita
{
///
/// A symbol with `arity() > 0`.
///
/// A function labels the internal (non-leaf) points of the parse trees that
/// represent the programs in the population. An example function set might be
/// {+, -, *}.
///
/// \warning
/// Each function should be able to handle gracefully all values it might
/// receive as input (this is called closure property). If there is a way to
/// crash the system, the GP system will certainly hit upon hit.
///
class function : public symbol
{
public:
  function(const std::string &, category_t, cvect);

  [[nodiscard]] category_t arg_category(std::size_t) const;

  [[nodiscard]] virtual bool associative() const;

  [[nodiscard]] virtual std::string display(format = c_format) const;

  [[nodiscard]] unsigned arity() const final;

  [[nodiscard]] bool is_valid() const override;

  [[nodiscard]] static const function *cast(const symbol *);

private:
  cvect argt_;
};

///
/// \return the number arguments of a funtion
///
inline unsigned function::arity() const
{
  Expects(argt_.size());
  return static_cast<unsigned>(argt_.size());
}

///
/// Is the symbol subject to the associative law of arithmetic?
///
/// \return `true` if the function is associative
///
/// `OP` is associative iff:
///
///     a OP (b OP c) = (a OP b) OP c = a OP b OP c
///
/// This information can be used for optimization and visualization.
///
/// \note
/// * Terminals haven't arguments and cannot be associative.
/// * Default (safe) value is `false`.
///
inline bool function::associative() const
{
  return false;
}

///
/// \param[in] i index of a function argument
/// \return      category of the `i`-th function argument
///
inline category_t function::arg_category(std::size_t i) const
{
  Expects(i < arity());
  return argt_[i];
}

///
/// This is a short cut function.
///
/// \param[in] s symbol pointer
/// \return      `s` casted to a vita::function pointer
///
inline const function *function::cast(const symbol *s)
{
  Expects(s->arity());
  return static_cast<const function *>(s);
}

}  // namespace vita

#endif  // include guard
