/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
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
///
/// A container for the symbols used by the GP engine.
///
/// Symbols are stored to be quickly recalled by category and randomly
/// selected.
///
/// \note
/// The functions and terminals used should be powerful enough to be able to
/// represent a solution to the problem. On the other hand, it's better not
/// to use a symbol set too large (this enlarges the search space and makes
/// harder the search for a solution).
///
class symbol_set
{
public:
  using weight_t = unsigned;

  symbol_set();

  void clear();

  symbol *insert(std::unique_ptr<symbol>, double = 1.0);
  template<class S, class ...Args> symbol *insert(double, Args &&...);
  template<class S, class ...Args> symbol *insert(Args &&...);

  const symbol &roulette(category_t) const;
  const terminal &roulette_terminal(category_t) const;

  const symbol &arg(std::size_t) const;

  std::vector<const symbol *> adts() const;
  void scale_adf_weights();

  symbol *decode(opcode_t) const;
  symbol *decode(const std::string &) const;

  category_t categories() const;
  unsigned terminals(category_t) const;

  weight_t weight(const symbol &) const;

  bool enough_terminals() const;
  bool debug() const;

  friend std::ostream &operator<<(std::ostream &, const symbol_set &);

private:
  // `arguments_` data member:
  // * is not present in the `collection` struct because an argument isn't
  //   bounded to a category (see `argument` class for more details);
  // * is not a subset of `symbols_` (the intersection of `arguments_` and
  //   `symbol_` is empty) because arguments aren't returned by the roulette
  //   functions.
  std::vector<std::unique_ptr<symbol>> arguments_;

  // This is the real, raw repository of symbols (it owns/stores the symbols).
  std::vector<std::unique_ptr<symbol>> symbols_;

  struct w_symbol
  {

    w_symbol(symbol *s, weight_t w) : sym(s), weight(w) { Expects(s); }

    bool operator==(w_symbol rhs) const
    {return sym == rhs.sym && weight == rhs.weight; }

    symbol *sym;

    /// Weight is used by the symbol_set::roulette method to control the
    /// probability of selection.
    weight_t weight;

    /// This is the default weight.
    enum : decltype(weight) {base_weight = 100};
  };

  // A collection is a structured-view on `symbols_` or on a subset of
  // `symbols_` (e.g. only on symbols of a specific category).
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
        Expects(!name_.empty());
      }

      void insert(const w_symbol &);

      std::size_t size() const { return elems_.size(); }
      const w_symbol &operator[](std::size_t i) const { return elems_[i]; }

      iterator begin() { return elems_.begin(); }
      const_iterator begin() const { return elems_.begin(); }
      iterator end() { return elems_.end(); }
      const_iterator end() const { return elems_.end(); }

      weight_t sum() const { return sum_; }

      template<class F> void scale_weights(double, F);
      const symbol &roulette() const;

      bool debug() const;

    private:
      sum_container_t elems_;

      // Sum of the weights of the symbols in the container.
      weight_t sum_;

      std::string name_;
    };

    explicit collection(std::string = "");

    sum_container       all;
    sum_container terminals;
    sum_container       adf;
    sum_container       adt;

    bool debug() const;

  private:
    std::string name_;
  };

  // The last element of the vector contains the category-agnostic view of
  // symbols:
  // - `views_.back().all.size()` is equal to the total number of symbols
  // - `views_[0].all.size()` is the number of symbols in category `0`
  std::vector<collection> views_;
};

///
/// Adds a symbol to the symbol set.
///
/// \tparam    S    symbol to be added
/// \param[in] wr   the weight of `S` (`1.0` means standard frequency, `2.0`
///                 double probability of selection)
/// \param[in] args arguments used to build `S`
/// \return         a raw pointer to the symbol just added (or `nullptr` in
///                 case of error)
///
/// Insert a symbol in the symbol set without the user having to allocate
/// memory. It doesn't completely replace the `insert(std::unique_ptr)`
/// method (e.g. building from factory).
///
template<class S, class ...Args> symbol *symbol_set::insert(double wr,
                                                            Args &&... args)
{
  return insert(std::make_unique<S>(args...), wr);
}

///
/// Adds a symbol to the symbol set.
///
/// \note Assumes a standard frequency (`1.0`) for symbol `S`.
///
template<class S, class ...Args> symbol *symbol_set::insert(Args &&... args)
{
  return insert(std::make_unique<S>(args...), 1.0);
}

std::ostream &operator<<(std::ostream &, const symbol_set &);

}  // namespace vita

#endif  // include guard
