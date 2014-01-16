/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(SYMBOL_SET_H)
#define      SYMBOL_SET_H

#include <string>
#include <vector>

#include "kernel/symbol.h"

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

    symbol *insert(std::unique_ptr<symbol>);

    symbol *roulette() const;
    symbol *roulette(category_t) const;
    symbol *roulette_terminal(category_t) const;

    symbol *arg(unsigned) const;

    symbol *get_adt(unsigned) const;
    unsigned adts() const;

    void reset_adf_weights();

    symbol *decode(opcode_t) const;
    symbol *decode(const std::string &) const;

    unsigned categories() const;
    unsigned terminals(category_t) const;

    bool enough_terminals() const;
    bool debug() const;

    friend std::ostream &operator<<(std::ostream &, const symbol_set &);

  private:  // Private data members.
    // arguments_ data member:
    // * is not present in the \a collection struct because an argument isn't
    //   bounded to a category (see \c argument constructor for more details);
    // * is not a subset of symbols_ (the intersection of arguments_ and
    //   symbol_ is an empty set) because arguments aren't returned by the
    //   roulette functions .
    std::vector<std::unique_ptr<symbol>> arguments_;

    std::vector<std::unique_ptr<symbol>>   symbols_;

    // Symbols of every category are inserted in this collection.
    struct collection
    {
      collection();

      std::vector<symbol *>   symbols;
      std::vector<symbol *> terminals;
      std::vector<symbol *>       adf;
      std::vector<symbol *>       adt;

      // The sum of the weights of all the symbols in the collection.
      std::uintmax_t sum;

      bool debug() const;
    } all_;

    // This struct contains all the symbols (\a all_) divided by category.
    struct by_category
    {
      explicit by_category(const collection & = collection());

      bool debug() const;

      std::vector<collection> category;
    } by_;
  };

  std::ostream &operator<<(std::ostream &, const symbol_set &);
}  // namespace vita

#endif  // SYMBOL_SET_H
