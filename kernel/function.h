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
  ///
  class function : public symbol
  {
  public:
    function(const std::string &, symbol_t, const std::vector<symbol_t> &,
             unsigned, bool = false);
    function(const std::string &, symbol_t, unsigned, unsigned = default_weight,
             bool = false);

    bool associative() const;
    bool parametric() const;

    symbol_t arg_type(unsigned) const;
    unsigned argc() const;

    bool check() const;

    static unsigned default_weight;

  private:
    symbol_t argt_[gene::k_args];
    unsigned               argc_;
    const bool      associative_;
  };
}  // namespace vita

#endif  // FUNCTION_H
