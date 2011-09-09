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
  class adf_0;
  class adf_n;
  class argument;
  class symbol;
  class terminal;
  class variable;

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
    ~symbol_set();

    void insert(symbol *const, bool);

    const symbol *roulette(bool = false) const;
    const argument *arg(unsigned) const;
    const symbol *get_special(unsigned) const;
    unsigned specials() const;
    const adf_0 *get_adf0(unsigned) const;

    void reset_adf_weights();

    const symbol *decode(unsigned) const;
    const symbol *decode(const std::string &) const;

    void delete_symbols();

    bool check() const;

  private:
    void clear();

    std::vector<symbol *>     symbols_;
    std::vector<terminal *> terminals_;
    std::vector<adf_n *>          adf_;
    std::vector<adf_0 *>         adf0_;
    std::vector<argument *> arguments_;
    std::vector<terminal *>  specials_;

    boost::uint64_t sum_;
  };
}  // namespace vita

#endif  // SYMBOL_SET_H
