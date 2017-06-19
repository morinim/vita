/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_TERMINAL_H)
#define      VITA_TERMINAL_H

#include "kernel/symbol.h"

namespace vita
{
///
/// A symbol with zero-arity.
///
/// A terminal might be a variable (input to the program), a constant value
/// or a function taking no arguments (e.g. move-north).
///
class terminal : public symbol
{
public:
  using symbol::symbol;

  unsigned arity() const final;

  virtual bool parametric() const;

  virtual double init() const;

  virtual std::string display(double, format = c_format) const;

  bool debug() const override;

  static const terminal *cast(const symbol *);
};

///
/// \return `0` (this is a terminal!)
///
inline unsigned terminal::arity() const
{
  return 0;
}

///
/// A parametric terminal needs an additional parameter to be evaluated.
///
/// \return `true` for parametric symbols
///
/// Genes associated with parametric symbols store an additional parameter
/// fetched at run-time and used for symbol evaluation.
///
inline bool terminal::parametric() const
{
  return false;
}

///
/// Used to initialize the terminal's internal parameter.
///
/// \return `0.0`
///
/// \remark
/// Derived classes should redefine this member function in a meaningful way.
///
inline double terminal::init() const
{
  return 0.0;
}

///
/// This is a short cut function.
///
/// \param[in] s symbol pointer
/// \return      `s` casted to a vita::terminal pointer
///
inline const terminal *terminal::cast(const symbol *s)
{
  Expects(s->arity() == 0);
  return static_cast<const terminal *>(s);
}

}  // namespace vita

#endif  // include guard
