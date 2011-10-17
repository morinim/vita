/**
 *
 *  \file symbol_set.h
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

#if !defined(SYMBOL_SET_H)
#define      SYMBOL_SET_H

#include <string>
#include <vector>

#include "kernel/vita.h"

namespace vita
{
  class argument;

  ///
  /// This is a container for the symbol set. Symbols are stored to be quickly
  /// recalled by type and randomly extracted.
  /// Memory management for symbols is a user's responsibility (anyway to help
  /// in this task there is the delete_symbols() function).
  /// The function and terminals used should be powerful enough to be able to
  /// represent a solution to the problem. On the other hand, it is better not
  /// to use too large a symbol set (this enalarges the search space and can
  /// sometimes make the search for a solution harder).
  ///
  class symbol_set
  {
  public:
    symbol_set();

    void insert(symbol_ptr, bool);

    const symbol_ptr &roulette(bool = false) const;
    const symbol_ptr &arg(unsigned) const;
    const symbol_ptr &get_adt(unsigned) const;
    unsigned adts() const;
    const symbol_ptr &get_special(unsigned) const;
    unsigned specials() const;

    void reset_adf_weights();

    symbol_ptr decode(unsigned) const;
    symbol_ptr decode(const std::string &) const;

    bool check() const;

  private:
    void clear();

    std::vector<symbol_ptr>   symbols_;
    std::vector<symbol_ptr> terminals_;
    std::vector<symbol_ptr>       adf_;
    std::vector<symbol_ptr>       adt_;
    std::vector<symbol_ptr> arguments_;

    // A special symbol is not used during initial random generation but it's
    // inserted at the end of the genome in a protected area. Terminals only
    // can be special.
    std::vector<symbol_ptr>  specials_;

    // The sum of the weights of all the symbols in the symbol set that are NOT
    // special.
    boost::uint64_t sum_;
  };
}  // namespace vita

#endif  // SYMBOL_SET_H
