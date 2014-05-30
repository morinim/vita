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

#if !defined(VITA_FUNCTION_H)
#define      VITA_FUNCTION_H

#include <string>
#include <vector>

#include "kernel/gene.h"

namespace vita
{
  ///
  /// \brief A symbol used in GP
  ///
  /// A \a function labels the internal (non-leaf) points of the parse trees
  /// that represent the programs in the \a population. An example function set
  /// might be {+,-,*}.
  /// Each function should be able to handle gracefully all values it might
  /// receive as input (this is called closure property).
  ///
  /// \warning
  /// If there is a way to crash the system, the GP system will certainly hit
  /// upon hit.
  ///
  class function : public symbol
  {
  public:
    function(const std::string &, category_t, const std::vector<category_t> &,
             unsigned = k_base_weight, bool = false);

    ///
    /// \param[in] i index of a function argument.
    /// \return category of the i-th function argument.
    ///
    category_t arg_category(unsigned i) const
    { assert(i < gene::k_args); return argt_[i]; }

    virtual bool debug() const override;

    ///
    /// \param[in] s symbol pointer.
    /// \return \a s casted to a vita::function pointer.
    ///
    /// This is a short cut function.
    ///
    static const function *cast(const symbol *s)
    {
      assert(s->arity());
      return static_cast<const function *>(s);
    }

  private:  // Private data members.
    category_t argt_[gene::k_args];
  };
}  // namespace vita

#if defined(VITA_NO_LIB)
#  include "function.cc"
#endif

#endif  // Include guard
