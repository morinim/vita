/**
 *
 *  \file symbol_set.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/symbol_set.h"
#include "kernel/adf.h"
#include "kernel/argument.h"
#include "kernel/random.h"

namespace vita
{
  namespace
  {
    ///
    /// \param[in] symbols set of symbols.
    /// \param[in] sum sum of the weights of the elements contained in
    ///                \a symbols.
    /// \return a random symbol from \a symbols.
    ///
    /// Probably the fastest way to produce a realization of a random variable
    /// X in a computer is to create a big table where each outcome \a i is
    /// inserterted a number of times proportional to P(X=i).
    ///
    /// Two fast methods are described in "Fast Generation of Discrete Random
    /// Variables" (Marsaglia, Tsang, Wang).
    /// Also boost::random::discrete_distribution seems quite fast.
    ///
    /// Anyway we choose the "roulette algorithm" because it's very simple and
    /// allows changing weights dynamically (performance differences can hardly
    /// be measured).
    /// \see http://en.wikipedia.org/wiki/Fitness_proportionate_selection
    ///
    symbol *roulette_(const std::vector<symbol *> &symbols, unsigned sum)
    {
      const auto slot(random::sup(sum));

      size_t i(0);
      for (auto wedge(symbols[i]->weight);
           wedge <= slot;
           wedge += symbols[++i]->weight)
      {}

      // This is a different approach from Eli Bendersky
      // (http://eli.thegreenplace.net):
      //
      //     unsigned total(0);
      //     size_t winner(0);
      //
      //     for (size_t i(0); i < symbols.size(); ++i)
      //     {
      //       total += symbols[i]->weight;
      //       if (random::sup(total + 1) < symbols[i]->weight)
      //         winner = i;
      //     }
      //     return winner;
      //
      // The interesting property of this algorithm is that you don't need to
      // know the sum of weights in advance in order to use it. The method is
      // cool, but slower than the standard roulette.

      assert(i < symbols.size());
      return symbols[i];
    }
  }  // anonymous namespace

  ///
  /// Sets up the object.
  /// The constructor allocates memory for up to \a k_args argument.
  ///
  symbol_set::symbol_set() : arguments_(gene::k_args)
  {
    for (unsigned i(0); i < gene::k_args; ++i)
      arguments_[i] = make_unique<argument>(i);

    assert(debug());
  }

  ///
  /// \param[in] n index of an argument symbol.
  /// \return a pointer to the n-th argument symbol.
  ///
  symbol *symbol_set::arg(unsigned n) const
  {
    assert(n < gene::k_args);
    return arguments_[n].get();
  }

  ///
  /// \param[in] i index of an ADT symbol.
  /// \return a pointer to the i-th ADT symbol.
  ///
  symbol *symbol_set::get_adt(unsigned i) const
  {
    assert(i < all_.adt.size());
    return all_.adt[i];
  }

  ///
  /// \return the number of ADT functions stored.
  ///
  unsigned symbol_set::adts() const
  {
    return all_.adt.size();
  }

  ///
  /// \param[in] i symbol to be added.
  /// \return a raw pointer to the symbol just added (or nullptr in case of
  ///         error).
  ///
  /// Adds a new \a symbol to the set. We manage to sort the symbols in
  /// descending order, with respect to the weight, so the selection algorithm
  /// would run faster.
  ///
  symbol *symbol_set::insert(std::unique_ptr<symbol> i)
  {
    assert(i);
    assert(i->weight);
    assert(i->debug());

    const auto raw(i.get());

    symbols_.push_back(std::move(i));

    all_.symbols.push_back(raw);
    all_.sum += raw->weight;

    if (raw->terminal())
    {
      all_.terminals.push_back(raw);

      if (raw->auto_defined())
        all_.adt.push_back(raw);
    }
    else  // not a terminal
      if (raw->auto_defined())
        all_.adf.push_back(raw);

    by_ = by_category(all_);

    std::sort(all_.symbols.begin(), all_.symbols.end(),
              [](const symbol *s1, const symbol *s2)
              { return s1->weight > s2->weight; });

    return raw;
  }

  ///
  void symbol_set::reset_adf_weights()
  {
    for (size_t i(0); i < adts(); ++i)
    {
      const auto w(all_.adt[i]->weight);
      const auto delta(w >  1 ? w/2 :
                       w == 1 ?   1 : 0);
      all_.sum -= delta;
      all_.adt[i]->weight -= delta;

      if (delta && all_.adt[i]->weight == 0)
      {
        for (size_t j(0); j < all_.terminals.size(); ++j)
          if (all_.terminals[j]->opcode() == all_.adt[i]->opcode())
          {
            all_.terminals.erase(all_.terminals.begin() + j);
            break;
          }

        for (size_t j(0); j < all_.symbols.size(); ++j)
          if (all_.symbols[j]->opcode() == all_.adt[i]->opcode())
          {
            all_.symbols.erase(all_.symbols.begin() + j);
            break;
          }
      }
    }

    for (size_t i(0); i < all_.adf.size(); ++i)
    {
      const auto w(all_.adf[i]->weight);
      const auto delta(w >  1 ? w/2 :
                       w == 1 ?   1 : 0);
      all_.sum -= delta;
      all_.adf[i]->weight -= delta;
    }

    by_ = by_category(all_);
  }

  ///
  /// \param[in] c a category.
  /// \return a random terminal of category \a c.
  ///
  symbol *symbol_set::roulette_terminal(category_t c) const
  {
    assert(c < categories());

    return random::element(by_.category[c].terminals);
  }

  ///
  /// \param[in] c a category.
  /// \return a random symbol of category \a c.
  ///
  symbol *symbol_set::roulette(category_t c) const
  {
    return roulette_(by_.category[c].symbols, by_.category[c].sum);
  }

  ///
  /// \return a random symbol from the set of all symbols.
  ///
  symbol *symbol_set::roulette() const
  {
    return roulette_(all_.symbols, all_.sum);
  }

  ///
  /// \param[in] opcode numerical code used as primary key for a symbol.
  /// \return a pointer to the \c vita::symbol identified by 'opcode'
  ///         (\c nullptr if not found).
  ///
  symbol *symbol_set::decode(opcode_t opcode) const
  {
    for (size_t i(0); i < all_.symbols.size(); ++i)
      if (all_.symbols[i]->opcode() == opcode)
        return all_.symbols[i];

    return nullptr;
  }

  ///
  /// \param[in] dex the name of a symbol.
  /// \return a pointer to the \c symbol identified by 'dex' (0 if not found).
  ///
  /// \attention Please note that opcodes (automatically assigned) are primary
  /// keys for symbols. Conversely the name of a symbol is choosen by the
  /// user, so, if you don't pay attention, different symbols may have the same
  /// name.
  ///
  symbol *symbol_set::decode(const std::string &dex) const
  {
    assert(dex != "");

    for (size_t i(0); i < all_.symbols.size(); ++i)
      if (all_.symbols[i]->display() == dex)
        return all_.symbols[i];

    return nullptr;
  }

  ///
  /// \return number of categories in the symbol set (>= 1).
  ///
  /// See also \c data::categories().
  ///
  unsigned symbol_set::categories() const
  {
    return by_.category.size();
  }

  ///
  /// \param[in] c a category.
  /// \return number of terminals in category \a c.
  ///
  unsigned symbol_set::terminals(category_t c) const
  {
    assert(c < by_.category.size());
    return by_.category[c].terminals.size();
  }

  ///
  /// \return \c true if there are enough terminals for secure individual
  ///         generation.
  ///
  /// We want at least one terminal for every category.
  ///
  bool symbol_set::enough_terminals() const
  {
    std::set<category_t> need;

    for (size_t i(0); i < all_.symbols.size(); ++i)
      for (size_t j(0); j < all_.symbols[i]->arity(); ++j)
        need.insert(function::cast(all_.symbols[i])->arg_category(j));

    for (const auto &cat : need)
    {
      const collection &cc(by_.category[cat]);

      if (cat >= categories() || !cc.terminals.size())
        return false;
    }
    return true;
  }

  ///
  /// \param[out] o output stream.
  /// \param[in] ss symbol set to be printed.
  /// \return output stream including \a ss.
  ///
  /// Useful for debugging purpouse.
  ///
  std::ostream &operator<<(std::ostream &o, const symbol_set &ss)
  {
    for (const symbol *s : ss.all_.symbols)
    {
      o << s->display();

      const auto arity(s->arity());
      if (arity)
        o << '(';
      for (size_t j(0); j < arity; ++j)
        o << function::cast(s)->arg_category(j)
          << (j+1 == arity ? "" : ", ");
      if (arity)
        o << ')';

        o << " -> " << s->category() << " (opcode " << s->opcode()
          << ", parametric " << s->parametric()
          << ", weight " << s->weight << ")" << std::endl;
    }

    return o << "Sum: " << ss.all_.sum << std::endl;
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol_set::debug() const
  {
    if (!all_.debug())
      return false;

    for (size_t i(0); i < by_.category.size(); ++i)
      if (!by_.category[i].debug())
        return false;

    return enough_terminals();
  }

  ///
  /// New empty collection.
  ///
  symbol_set::collection::collection()
  {
    symbols.clear();
    terminals.clear();
    adf.clear();
    adt.clear();

    sum = 0;
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol_set::collection::debug() const
  {
    std::uintmax_t check_sum(0);

    for (size_t j(0); j < symbols.size(); ++j)
    {
      if (!symbols[j]->debug())
        return false;

      check_sum += symbols[j]->weight;

      if (symbols[j]->weight == 0)
        return false;

      bool found(false);
      if (symbols[j]->terminal())
      {
        // Terminals must be in the terminals_ vector.
        for (size_t i(0); i < terminals.size() && !found; ++i)
          found = (symbols[j] == terminals[i]);

        if (symbols[j]->auto_defined())
          for (size_t i(0); i < adt.size() && !found; ++i)
            found = (symbols[j] == adt[i]);
      }
      else  // Function
        if (symbols[j]->auto_defined())
          for (size_t i(0); i < adf.size() && !found; ++i)
            found = (symbols[j] == adf[i]);
        else
          found = true;

      if (!found)
        return false;
    }

    if (check_sum != sum)
      return false;

    // There should be one terminal at least.
    return symbols.size() == 0 || terminals.size() > 0;
  }

  ///
  /// \param[in] c a collection containing many categories of symbol.
  ///
  /// Initialize the struct using collection \a c as input parameter (\a c
  /// should be a collection containing more than one category).
  ///
  symbol_set::by_category::by_category(const collection &c)
  {
    category.clear();

    for (size_t i(0); i < c.symbols.size(); ++i)
    {
      const category_t cat(c.symbols[i]->category());
      if (cat >= category.size())
      {
        category.resize(cat + 1);
        category[cat] = collection();
      }

      category[cat].symbols.push_back(c.symbols[i]);
      category[cat].sum += c.symbols[i]->weight;
    }

    for (size_t i(0); i < c.terminals.size(); ++i)
      category[c.terminals[i]->category()].terminals.push_back(c.terminals[i]);

    for (size_t i(0); i < c.adf.size(); ++i)
      category[c.adf[i]->category()].adf.push_back(c.adf[i]);

    for (size_t i(0); i < c.adt.size(); ++i)
      category[c.adt[i]->category()].adt.push_back(c.adt[i]);

    for (size_t j(0); j < category.size(); ++j)
      std::sort(category[j].symbols.begin(), category[j].symbols.end(),
                [](const symbol *s1, const symbol *s2)
                { return s1->weight > s2->weight; });

    assert(debug());
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol_set::by_category::debug() const
  {
    for (size_t t(0); t < category.size(); ++t)
    {
      const size_t s(category[t].symbols.size());
      if (s < category[t].terminals.size())
        return false;
      if (s < category[t].adf.size())
        return false;
      if (s < category[t].adt.size())
        return false;

      if (!category[t].debug())
        return false;
    }

    return true;
  }
}  // Namespace vita
