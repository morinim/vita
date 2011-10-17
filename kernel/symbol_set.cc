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
  /// Utility function used to help the constructor and the delete_symbols
  /// member function in the clean up process.
  ///
  void symbol_set::clear()
  {
    adf_.clear();
    adt_.clear();
    symbols_.clear();
    terminals_.clear();
    specials_.clear();

    sum_ = 0;
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
    assert(i < adt_.size());
    return adt_[i];
  }

  ///
  /// \return the number of ADT functions stored.
  ///
  unsigned symbol_set::adts() const
  {
    return adt_.size();
  }

  ///
  /// \param[in] n index of a special \a symbol.
  /// \return a pointer to the n-th special \a symbol.
  ///
  const symbol_ptr &symbol_set::get_special(unsigned n) const
  {
    assert(n < specials_.size());
    return specials_[n];
  }

  ///
  /// \return the number of special symbols in the symbol set.
  ///
  unsigned symbol_set::specials() const
  {
    return specials_.size();
  }

  ///
  /// \param[in] i symbol to be added.
  /// \param[in] special if \c true the \a symbol is not used during initial
  ///            random generation but it's inserted at the end of the genome
  ///            in a protected area. Only terminals can be special.
  ///
  /// Adds a new \a symbol to the set.
  ///
  void symbol_set::insert(symbol_ptr i, bool special)
  {
    assert(i && i->weight && i->check());

    if (special)
    {
      assert(i->terminal());
      specials_.push_back(i);
    }
    else
    {
      symbols_.push_back(i);
      sum_ += i->weight;
    }

    if (i->terminal())
    {
      terminals_.push_back(i);

      if (i->auto_defined())
        adt_.push_back(i);
    }
    else  // not a terminal
      if (i->auto_defined())
        adf_.push_back(i);

    assert(check());
  }

  ///
  void symbol_set::reset_adf_weights()
  {
    for (unsigned i(0); i < adts(); ++i)
    {
      const unsigned w(adt_[i]->weight);
      const unsigned delta(w >  1 ? w/2 :
                           w == 1 ? 1 : 0);
      sum_ -= delta;
      adt_[i]->weight -= delta;

      if (delta && adt_[i]->weight == 0)
      {
        for (unsigned j(0); j < terminals_.size(); ++j)
          if (terminals_[j]->opcode() == adt_[i]->opcode())
          {
            terminals_.erase(terminals_.begin()+j);
            break;
          }

        for (unsigned j(0); j < symbols_.size(); ++j)
          if (symbols_[j]->opcode() == adt_[i]->opcode())
          {
            symbols_.erase(symbols_.begin()+j);
            break;
          }
      }
    }

    for (unsigned i(0); i < adf_.size(); ++i)
    {
      const unsigned w(adf_[i]->weight);
      const unsigned delta(w >  1 ? w/2 :
                           w == 1 ? 1 : 0);
      sum_ -= delta;
      adf_[i]->weight -= delta;
    }
  }

  ///
  /// \param[in] only_t if true extracts only terminals.
  /// \return a random symbol.
  ///
  /// If \a only_t == \c true extracts a \a terminal else a random \a symbol
  /// (may be a \a terminal, a primitive function or an ADF).
  ///
  const symbol_ptr &symbol_set::roulette(bool only_t) const
  {
    assert(sum_);

    if (only_t)
    {
      const unsigned i(random::between<unsigned>(0, terminals_.size()));

      assert(!dynamic_cast<argument *>(terminals_[i].get()));
      return terminals_[i];
    }

    const unsigned slot(random::between<unsigned>(0, sum_));

    unsigned i(0);
    for (unsigned wedge(symbols_[i]->weight);
         wedge <= slot && i+1 < symbols_.size();
         wedge += symbols_[++i]->weight)
    {}

    assert(!dynamic_cast<argument *>(symbols_[i].get()));
    return symbols_[i];
  }

  ///
  /// \param[in] opcode numerical code used as primary key for a symbol.
  /// \return a pointer to the \c symbol identified by 'opcode' (0 if not
  ///         found).
  ///
  symbol_ptr symbol_set::decode(unsigned opcode) const
  {
    for (unsigned i(0); i < symbols_.size(); ++i)
      if (symbols_[i]->opcode() == opcode)
        return symbols_[i];

    return symbol_ptr();
  }

  ///
  /// \param[in] dex the name of a symbol.
  /// \return a pointer to the \c symbol identified by 'dex' (0 if not found).
  ///
  /// Please note that opcode are primary key for symbols because they are
  /// automatically assigned. The name of a symbol is choosen by the user,
  /// so if you don't pay attention different symbols may have the same name.
  ///
  symbol_ptr symbol_set::decode(const std::string &dex) const
  {
    for (unsigned i(0); i < symbols_.size(); ++i)
      if (symbols_[i]->display() == dex)
        return symbols_[i];

    return symbol_ptr();
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool symbol_set::check() const
  {
    boost::uint64_t sum(0);

    for (unsigned j(0); j < symbols_.size(); ++j)
    {
      if (!symbols_[j]->check())
        return false;

      sum += symbols_[j]->weight;

      if (symbols_[j]->weight == 0)
        return false;

      bool found(false);
      if (symbols_[j]->terminal())
      {
        // Terminals must be in the _terminals vector.
        for (unsigned i(0); i < terminals_.size() && !found; ++i)
          found = (symbols_[j] == terminals_[i]);

        if (symbols_[j]->auto_defined())
          for (unsigned i(0); i < adts() && !found; ++i)
            found = (symbols_[j] == adt_[i]);
      }
      else  // Function
        if (symbols_[j]->auto_defined())
          for (unsigned i(0); i < adf_.size() && !found; ++i)
            found = (symbols_[j] == adf_[i]);
        else
          found = true;

      if (!found)
        return false;
    }

    if (sum != sum_)
      return false;

    // There should be one terminal at least.
    return symbols_.size() == 0 || terminals_.size() > 0;
  }
}  // Namespace vita
