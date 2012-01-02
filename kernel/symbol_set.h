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
  /// recalled by category and randomly extracted.
  /// The function and terminals used should be powerful enough to be able to
  /// represent a solution to the problem. On the other hand, it is better not
  /// to use too large a symbol set (this enalarges the search space and can
  /// sometimes make the search for a solution harder).
  ///
  class symbol_set
  {
  public:
    symbol_set();

    void insert(const symbol_ptr &);

    const symbol_ptr &roulette() const;
    const symbol_ptr &roulette(category_t) const;
    const symbol_ptr &roulette_terminal(category_t) const;

    const symbol_ptr &arg(unsigned) const;

    const symbol_ptr &get_adt(unsigned) const;
    unsigned adts() const;

    void reset_adf_weights();

    const symbol_ptr &decode(unsigned) const;
    const symbol_ptr &decode(const std::string &) const;

    unsigned categories() const;
    unsigned terminals(category_t) const;

    bool enough_terminals() const;
    bool check() const;

    friend std::ostream &operator<<(std::ostream &, const symbol_set &);

  private:
    static const symbol_ptr empty_ptr;

    typedef std::vector<symbol_ptr> s_vector;

    void clear();
    const symbol_ptr &roulette(const s_vector &, boost::uint64_t) const;

    // \a arguments_ is not included in the \a collection struct because
    // an argument isn't bounded to a category (see \c argument constructor for
    // more details).
    s_vector arguments_;

    // Symbols of every category are inserted in this collection.
    struct collection
    {
      collection();

      s_vector   symbols;
      s_vector terminals;
      s_vector       adf;
      s_vector       adt;

      // The sum of the weights of all the symbols in the collection.
      boost::uint64_t sum;

      bool check() const;
    } all_;

    // This struct contains all the symbols (\a all_) divided by category.
    struct by_category
    {
      explicit by_category(const collection & = collection());

      bool check() const;

      std::vector<collection> category;
    } by_;
  };

  std::ostream &operator<<(std::ostream &, const symbol_set &);
}  // namespace vita

#endif  // SYMBOL_SET_H
