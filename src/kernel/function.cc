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

#if !defined(VITA_NO_LIB)
#  include "kernel/function.h"
#endif

namespace vita
{
  ///
  /// \param[in] dis string representation of the function (e.g. for the plus
  ///                \a function it could by "ADD" or "+").
  /// \param[in] c category of the function (i.e. the category of the output
  ///              value).
  /// \param[in] args input parameters (type and number) of the function (in
  ///                 C++ they are called the "function signature").
  ///
  VITA_INLINE
  function::function(const std::string &dis, category_t c, cvect args)
    : symbol(dis, c), argt_(std::move(args))
  {
    arity_ = static_cast<decltype(arity_)>(argt_.size());

    assert(debug());
  }

  ///
  /// \return \c true if the \a function passes the internal consistency check.
  ///
  VITA_INLINE
  bool function::debug() const
  {
    if (arity() == 0)  // This is a function, we want some argument...
      return false;

    return symbol::debug();
  }
}  // namespace vita
