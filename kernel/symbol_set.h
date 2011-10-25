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

    bool enough_terminals() const;
    bool check() const;

  private:
    void clear();

    // \a arguments_ is not included in the \a collection struct because type
    // isn't fixed for an argument (see \c argument constructor for more
    // details).
    std::vector<symbol_ptr> arguments_;

    struct collection
    {
      std::vector<symbol_ptr>   symbols;
      std::vector<symbol_ptr> terminals;
      std::vector<symbol_ptr>       adf;
      std::vector<symbol_ptr>       adt;

      // A special symbol is not used during initial random generation but it's
      // inserted at the end of the genome in a protected area. Only terminals
      // can be special.
      std::vector<symbol_ptr>  specials;

      // The sum of the weights of all the symbols in the collection that are
      // NOT special.
      boost::uint64_t sum;

      bool check() const;
    };

    // Symbols of every type are inserted in this collection.
    collection all_;

    // This struct contains all the symbols (\a all_) divided by type.
    class by_type
    {
    public:
      explicit by_type(const collection & = collection());

      /// Number of different types loaded in the collection. Note that
      /// \c type() could be less than \c type.size()
      unsigned types() const { return n_types; }

      std::vector<collection> type;

      bool check() const;

    private:
      unsigned n_types;
    } by_;
  };
}  // namespace vita

#endif  // SYMBOL_SET_H
