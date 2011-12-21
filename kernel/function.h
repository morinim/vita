/**
 *
 *  \file function.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(FUNCTION_H)
#define      FUNCTION_H

#include <string>
#include <vector>

#include "kernel/vita.h"
#include "kernel/gene.h"
#include "kernel/symbol.h"

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
             unsigned = default_weight, bool = false);

    ///
    /// \return \c true if the function is associative (e.g. sum is associative,
    ///         division isn't).
    ///
    bool associative() const { return associative_; }

    ///
    /// \return \c false (function are never parametric).
    ///
    bool parametric() const { return false; }

    ///
    /// \param[in] index of a function argument.
    /// \return category of the i-th function argument.
    ///
    category_t arg_category(unsigned i) const
    { assert(i < gene::k_args); return argt_[i]; }

    ///
    /// \return the number of arguments (0 arguments => terminal).
    ///
    unsigned arity() const { assert(arity_); return arity_; }

    bool check() const;

    static unsigned default_weight;

  private:
    category_t argt_[gene::k_args];
    unsigned                arity_;
    const bool        associative_;
  };
}  // namespace vita

#endif  // FUNCTION_H
