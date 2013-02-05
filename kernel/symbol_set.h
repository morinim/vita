/**
 *
 *  \file symbol_set.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SYMBOL_SET_H)
#define      SYMBOL_SET_H

#include <string>
#include <vector>

#include "vita.h"

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

    const symbol_ptr &arg(size_t) const;

    const symbol_ptr &get_adt(size_t) const;
    size_t adts() const;

    void reset_adf_weights();

    const symbol_ptr &decode(opcode_t) const;
    const symbol_ptr &decode(const std::string &) const;

    size_t categories() const;
    size_t terminals(category_t) const;

    bool enough_terminals() const;
    bool debug() const;

    friend std::ostream &operator<<(std::ostream &, const symbol_set &);

  private:  // Private support functions.
    typedef std::vector<symbol_ptr> s_vector;

    static const symbol_ptr empty_ptr;

    void clear();
    const symbol_ptr &roulette(const s_vector &, std::uintmax_t) const;

  private:  // Private data members.
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
