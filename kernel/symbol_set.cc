/**
 *
 *  \file symbol_set.cc
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

#include "kernel/symbol_set.h"
#include "kernel/adf.h"
#include "kernel/argument.h"
#include "kernel/random.h"

namespace vita
{
  ///
  /// Sets up the object.
  /// The constructor allocates memory for up to \a k_args argument.
  ///
  symbol_set::symbol_set()
  {
    clear();

    for (unsigned i(0); i < gene::k_args; ++i)
      arguments_.push_back(std::shared_ptr<argument>(new argument(i)));

    assert(check());
  }

  ///
  /// Utility function used to help the constructor in the clean up process.
  ///
  void symbol_set::clear()
  {
    all_ = collection();
    by_ = by_category();
  }

  ///
  /// \param[in] n index of an argument symbol.
  /// \return a pointer to the n-th argument symbol.
  ///
  const symbol_ptr &symbol_set::arg(unsigned n) const
  {
    assert(n < gene::k_args);
    return arguments_[n];
  }

  ///
  /// \param[in] i index of an ADT symbol.
  /// \return a pointer to the i-th ADT symbol.
  ///
  const symbol_ptr &symbol_set::get_adt(unsigned i) const
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
  /// \param[in] n index of a sticky \a symbol.
  /// \return a pointer to the n-th sticky \a symbol.
  ///
  const symbol_ptr &symbol_set::get_sticky(unsigned n) const
  {
    assert(n < all_.stickies.size());
    return all_.stickies[n];
  }

  ///
  /// \return the number of sticky symbols in the symbol set.
  ///
  unsigned symbol_set::stickies() const
  {
    return all_.stickies.size();
  }

  ///
  /// \param[in] i symbol to be added.
  /// \param[in] sticky if \c true the \a symbol is not used during initial
  ///            random generation but it's inserted at the end of the genome
  ///            in a protected area. Only terminals can be sticky.
  ///
  /// Adds a new \a symbol to the set.
  ///
  void symbol_set::insert(symbol_ptr i, bool sticky)
  {
    assert(i && i->weight && i->check());

    // Stickies aren't inserted in the symbols' vector.
    if (sticky)
    {
      assert(i->terminal());
      all_.stickies.push_back(i);
    }
    else
    {
      all_.symbols.push_back(i);
      all_.sum += i->weight;
    }

    if (i->terminal())
    {
      all_.terminals.push_back(i);

      if (i->auto_defined())
        all_.adt.push_back(i);
    }
    else  // not a terminal
      if (i->auto_defined())
        all_.adf.push_back(i);

    by_ = by_category(all_);
  }

  ///
  void symbol_set::reset_adf_weights()
  {
    for (unsigned i(0); i < adts(); ++i)
    {
      const unsigned w(all_.adt[i]->weight);
      const unsigned delta(w >  1 ? w/2 :
                           w == 1 ? 1 : 0);
      all_.sum -= delta;
      all_.adt[i]->weight -= delta;

      if (delta && all_.adt[i]->weight == 0)
      {
        for (unsigned j(0); j < all_.terminals.size(); ++j)
          if (all_.terminals[j]->opcode() == all_.adt[i]->opcode())
          {
            all_.terminals.erase(all_.terminals.begin()+j);
            break;
          }

        for (unsigned j(0); j < all_.symbols.size(); ++j)
          if (all_.symbols[j]->opcode() == all_.adt[i]->opcode())
          {
            all_.symbols.erase(all_.symbols.begin()+j);
            break;
          }
      }
    }

    for (unsigned i(0); i < all_.adf.size(); ++i)
    {
      const unsigned w(all_.adf[i]->weight);
      const unsigned delta(w >  1 ? w/2 :
                           w == 1 ? 1 : 0);
      all_.sum -= delta;
      all_.adf[i]->weight -= delta;
    }

    by_ = by_category(all_);
  }

  ///
  /// \param[in] only_t if \c true extracts only terminals.
  /// \return a random symbol.
  ///
  /// If \a only_t == \c true extracts a \a terminal else a random \a symbol
  /// (may be a terminal, a primitive function or an ADF).
  ///
  const symbol_ptr &symbol_set::roulette(bool only_t) const
  {
    assert(all_.sum);

    if (only_t)
    {
      const unsigned i(random::between<unsigned>(0, all_.terminals.size()));

      assert(!dynamic_cast<argument *>(all_.terminals[i].get()));
      return all_.terminals[i];
    }

    const unsigned slot(random::between<unsigned>(0, all_.sum));

    unsigned i(0);
    for (unsigned wedge(all_.symbols[i]->weight);
         wedge <= slot && i+1 < all_.symbols.size();
         wedge += all_.symbols[++i]->weight)
    {}

    assert(!dynamic_cast<argument *>(all_.symbols[i].get()));
    return all_.symbols[i];
  }

  ///
  /// \param[in] opcode numerical code used as primary key for a symbol.
  /// \return a pointer to the \c vita::symbol identified by 'opcode'
  ///         (\c nullptr if not found).
  ///
  symbol_ptr symbol_set::decode(unsigned opcode) const
  {
    for (unsigned i(0); i < all_.symbols.size(); ++i)
      if (all_.symbols[i]->opcode() == opcode)
        return all_.symbols[i];

    return symbol_ptr();
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
  symbol_ptr symbol_set::decode(const std::string &dex) const
  {
    assert(dex != "");

    for (unsigned i(0); i < all_.symbols.size(); ++i)
      if (all_.symbols[i]->display() == dex)
        return all_.symbols[i];

    return symbol_ptr();
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
  /// \return \c true if there are enough terminals for secure individual
  ///         generation.
  ///
  /// We want at least one terminal for every category.
  ///
  bool symbol_set::enough_terminals() const
  {
    std::set<category_t> need;

    for (unsigned i(0); i < all_.symbols.size(); ++i)
      for (unsigned j(0); j < all_.symbols[i]->arity(); ++j)
        need.insert(static_cast<function *>(all_.symbols[i].get())
                    ->arg_category(j));

    for (auto cat(need.begin()); cat != need.end(); ++cat)
      if (*cat >= categories() ||
          (!by_.category[*cat].terminals.size() &&
           !by_.category[*cat].stickies.size()))
        return false;

    return true;
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol_set::check() const
  {
    if (!all_.check())
      return false;

    for (unsigned i(0); i < by_.category.size(); ++i)
      if (!by_.category[i].check())
        return false;

    return enough_terminals();
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol_set::collection::check() const
  {
    boost::uint64_t sum(0);

    for (unsigned j(0); j < symbols.size(); ++j)
    {
      if (!symbols[j]->check())
        return false;

      sum += symbols[j]->weight;

      if (symbols[j]->weight == 0)
        return false;

      bool found(false);
      if (symbols[j]->terminal())
      {
        // Terminals must be in the _terminals vector.
        for (unsigned i(0); i < terminals.size() && !found; ++i)
          found = (symbols[j] == terminals[i]);

        if (symbols[j]->auto_defined())
          for (unsigned i(0); i < adt.size() && !found; ++i)
            found = (symbols[j] == adt[i]);
      }
      else  // Function
        if (symbols[j]->auto_defined())
          for (unsigned i(0); i < adf.size() && !found; ++i)
            found = (symbols[j] == adf[i]);
        else
          found = true;

      if (!found)
        return false;
    }

    if (sum != sum)
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

    for (unsigned i(0); i < c.symbols.size(); ++i)
    {
      const category_t cat(c.symbols[i]->category());
      if (cat >= category.size())
        category.resize(cat + 1);

      category[cat].symbols.push_back(c.symbols[i]);
      category[cat].sum += c.symbols[i]->weight;
    }

    for (unsigned i(0); i < c.terminals.size(); ++i)
      category[c.terminals[i]->category()].terminals.push_back(c.terminals[i]);

    for (unsigned i(0); i < c.adf.size(); ++i)
      category[c.adf[i]->category()].adf.push_back(c.adf[i]);

    for (unsigned i(0); i < c.adt.size(); ++i)
      category[c.adt[i]->category()].adt.push_back(c.adt[i]);

    for (unsigned i(0); i < c.stickies.size(); ++i)
    {
      const unsigned cat(c.stickies[i]->category());
      if (cat >= category.size())
        category.resize(cat + 1);

      category[cat].stickies.push_back(c.stickies[i]);
    }

    //for (unsigned i(0); i < category.size(); ++i)
    //  if (category[i].symbols.size() || category[i].stickies.size())
    //    ++n_categories;

    assert(check());
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol_set::by_category::check() const
  {
    for (unsigned t(0); t < category.size(); ++t)
    {
      const unsigned s(category[t].symbols.size());
      if (s < category[t].terminals.size() ||
          s < category[t].adf.size() ||
          s < category[t].adt.size())
        return false;
    }

    return true;
  }
}  // Namespace vita
