/**
 *
 *  \file function.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
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
  /// \param[in] w the weight of the function (used for random initialization).
  /// \param[in] asve \c true if the function is associative (e.g. addition is
  ///                 associative, division isn't).
  ///
  VITA_INLINE
  function::function(const std::string &dis, category_t c,
                     const std::vector<category_t> &args, unsigned w,
                     bool asve) : symbol(dis, c, w), associative_(asve)
  {
    assert(args.size() <= gene::k_args);

    for (arity_ = 0; arity_ < args.size(); ++arity_)
      argt_[arity_] = args[arity_];

    // for (size_t i(arity()); i < gene::k_args; ++i)
    //   argt_[i] = std::numeric_limits<category_t>::max();

    assert(debug());
  }

  ///
  /// \return \c true if the \a function passes the internal consistency check.
  ///
  VITA_INLINE
  bool function::debug() const
  {
    return arity_ && arity_ <= gene::k_args && symbol::debug();
  }
}  // Namespace vita
