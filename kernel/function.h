/**
 *
 *  \file function.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(FUNCTION_H)
#define      FUNCTION_H

#include <string>
#include <vector>

#include "gene.h"
#include "symbol.h"

namespace vita
{
  ///
  /// A symbol used in GP. A \a function label the internal (non-leaf) points
  /// of the parse trees that represent the programs in the \a population. An
  /// example function set might be {+,-,*}.
  /// Each function should be able to handle gracefully all values it might
  /// receive as input (this is called closure property). Remember: if there is
  /// a way to crash the system, the GP system will certainly hit upon hit.
  ///
  class function : public symbol
  {
  public:
    function(const std::string &, category_t, const std::vector<category_t> &,
             unsigned = k_base_weight, bool = false);

    ///
    /// \return \c true if the function is associative (e.g. sum is associative,
    ///         division isn't).
    ///
    virtual bool associative() const { return associative_; }

    ///
    /// \return \c false (function are never parametric).
    ///
    virtual bool parametric() const { return false; }

    ///
    /// \param[in] i index of a function argument.
    /// \return category of the i-th function argument.
    ///
    category_t arg_category(unsigned i) const
    { assert(i < gene::k_args); return argt_[i]; }

    ///
    /// \return the number of arguments (0 arguments => terminal).
    ///
    virtual size_t arity() const { assert(arity_); return arity_; }

    virtual bool debug() const;

    ///
    /// \param[in] s symbol pointer.
    /// \return \a s casted to a vita::function pointer.
    ///
    /// This is a short cut function.
    ///
    static const function *cast(const symbol::ptr &s)
    {
      assert(s->arity());
      return static_cast<function *>(s.get());
    }

  private:  // Private data members.
    category_t argt_[gene::k_args];
    size_t                  arity_;
    bool              associative_;
  };
}  // namespace vita

#endif  // FUNCTION_H
