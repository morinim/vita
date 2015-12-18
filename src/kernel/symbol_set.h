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
/// \brief A container for the symbols used by the GP engine
///
/// Symbols are stored to be quickly recalled by category and randomly
/// selected.
///
/// \note
/// The functions and terminals used should be powerful enough to be able to
/// represent a solution to the problem. On the other hand, it is better not
/// to use a symbol set too large (this enlarges the search space and makes
/// harder the search for a solution).
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

  symbol *get_adt(std::size_t) const;
  std::size_t adts() const;
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
  class collection
  {
  public:
    class sum_container
    {
    public:
      using sum_container_t = std::vector<w_symbol>;
      using iterator = sum_container_t::iterator;
      using const_iterator = sum_container_t::const_iterator;

      explicit sum_container(std::string n)
        : elems_(), sum_(0), name_(std::move(n))
      {
        assert(!name_.empty());
      }

      void insert(const w_symbol &);

      std::size_t size() const { return elems_.size(); }
      const w_symbol &operator[](std::size_t i) const { return elems_[i]; }

      iterator begin() { return elems_.begin(); }
      const_iterator begin() const { return elems_.begin(); }
      iterator end() { return elems_.end(); }
      const_iterator end() const { return elems_.end(); }
      iterator erase(iterator first, iterator last)
      { return elems_.erase(first, last); }

      unsigned sum() const { return sum_; }
      void changed_weight(unsigned d) { sum_ += d; };

      symbol *roulette() const;

      bool debug() const;

    private:
      sum_container_t elems_;

      /// Sum of the weights of the symbols in the container.
      unsigned sum_;

      std::string name_;
    };

    explicit collection(std::string = "");

    sum_container   symbols;
    sum_container terminals;
    sum_container       adf;
    sum_container       adt;

    bool debug() const;

  private:
    std::string name_;
  } all_;

  // This struct contains all the symbols (`all_`) divided by category.
  struct by_category
  {
    by_category() : category() {}
    explicit by_category(const collection &);

    bool debug() const;

    std::vector<collection> category;
  } by_;
};

std::ostream &operator<<(std::ostream &, const symbol_set &);

}  // namespace vita

#endif  // include guard
