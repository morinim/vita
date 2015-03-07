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

#if !defined(VITA_FUNCTION_H)
#define      VITA_FUNCTION_H

#include <string>

#include "kernel/symbol.h"

namespace vita
{
///
/// \brief A symbol (arity > 0) used in GP
///
/// A function labels the internal (non-leaf) points of the parse trees that
/// represent the programs in the population. An example function set might be
/// {+, -, *}.
///
/// \warning
/// Each function should be able to handle gracefully all values it might
/// receive as input (this is called closure property).
/// If there is a way to crash the system, the GP system will certainly hit
/// upon hit.
///
class function : public symbol
{
public:
  function(const std::string &, category_t, cvect);

  category_t arg_category(unsigned) const;

  virtual unsigned arity() const override;

  virtual bool debug() const override;

  static const function *cast(const symbol *);

private:  // Private data members
  cvect argt_;
};

///
/// \return the number arguments of a funtion.
///
inline unsigned function::arity() const
{
  assert(argt_.size());
  return static_cast<unsigned>(argt_.size());
}

///
/// \param[in] i index of a function argument.
/// \return category of the i-th function argument.
///
inline category_t function::arg_category(unsigned i) const
{
  assert(i < arity());
  return argt_[i];
}

///
/// \param[in] s symbol pointer.
/// \return `s` casted to a vita::function pointer.
///
/// This is a short cut function.
///
inline const function *function::cast(const symbol *s)
{
  assert(s->arity());
  return static_cast<const function *>(s);
}

}  // namespace vita

#if defined(VITA_NO_LIB)
#  include "kernel/function.cc"
#endif

#endif  // Include guard
