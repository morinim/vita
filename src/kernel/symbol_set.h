/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SYMBOL_SET_H)
#define      VITA_SYMBOL_SET_H

#include <string>

#include "kernel/terminal.h"

namespace vita
{
struct w_symbol
{
  w_symbol(symbol *s, unsigned w) : sym(s), weight(w) { assert(s); }

  bool operator==(w_symbol rhs) const
  {return sym == rhs.sym && weight == rhs.weight; }

  symbol *sym;

  /// Weight is used by the symbol_set::roulette method to control the
  /// probability of selection for the symbol.
  unsigned weight;

  /// This is the default weight.
  static constexpr decltype(weight) base_weight{100};
};

///
/// This is a container for the symbol set. Symbols are stored to be quickly
/// recalled by category and randomly extracted.
/// The functions and terminals used should be powerful enough to be able to
/// represent a solution to the problem. On the other hand, it is better not
/// to use a symbol set too large (this enlarges the search space and can
/// sometimes make the search for a solution harder).
///
class symbol_set
{
public:
  symbol_set();

  void clear();

  symbol *insert(std::unique_ptr<symbol>, double = 1.0);

  symbol *roulette() const;
  symbol *roulette(category_t) const;
  terminal *roulette_terminal(category_t) const;

  symbol *arg(unsigned) const;

  symbol *get_adt(unsigned) const;
  unsigned adts() const;
  void reset_adf_weights();

  symbol *decode(opcode_t) const;
  symbol *decode(const std::string &) const;

  unsigned categories() const;
  unsigned terminals(category_t) const;

  unsigned weight(const symbol *) const;

  bool enough_terminals() const;
  bool debug() const;

  friend std::ostream &operator<<(std::ostream &, const symbol_set &);

private:
  // `arguments_` data member:
  // * is not present in the `collection` struct because an argument isn't
  //   bounded to a category (see `argument` class for more details);
  // * is not a subset of `symbols_` (the intersection of `arguments_` and
  //   symbol_ is an empty set) because arguments aren't returned by the
  //   roulette functions .
  std::vector<std::unique_ptr<symbol>> arguments_;

  // This is the real, raw repository of symbols (it owns/stores the symbols).
  std::vector<std::unique_ptr<symbol>> symbols_;

  // A collection is a structured-view on `symbols_` (the `all_` variable) or
  // on a subset of `symbols_` (e.g. only on symbols of a specific category).
  struct collection
  {
    collection();

    std::vector<w_symbol>   symbols;
    std::vector<w_symbol> terminals;
    std::vector<w_symbol>       adf;
    std::vector<w_symbol>       adt;

    unsigned sum;    // sum of the weights of all the symbols in the collection
    unsigned sum_t;  // sum of the weights of the terminals in the collection

    bool debug(std::string) const;
  } all_;

  // This struct contains all the symbols (`all_`) divided by category.
  struct by_category
  {
    explicit by_category(const collection & = collection());

    bool debug() const;

    std::vector<collection> category;
  } by_;
};

std::ostream &operator<<(std::ostream &, const symbol_set &);
}  // namespace vita

#endif  // include guard
