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
  function::function(const std::string &dis, category_t c,
                     const std::vector<category_t> &args)
    : symbol(dis, c)
  {
    assert(args.size() <= gene::k_args);

    arity_ = static_cast<decltype(arity_)>(args.size());

    for (auto i(decltype(arity_){0}); i < arity_; ++i)
      argt_[i] = args[i];

    // for (auto i(arity()); i < gene::k_args; ++i)
    //   argt_[i] = std::numeric_limits<category_t>::max();

    assert(debug());
  }

  ///
  /// \return \c true if the \a function passes the internal consistency check.
  ///
  VITA_INLINE
  bool function::debug() const
  {
    if (!arity_)  // This is a function, we want some argument...
      return false;

    if (arity_ > gene::k_args)  // ... but not to much!
      return false;

    return symbol::debug();
  }
}  // namespace vita
