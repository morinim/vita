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

#include "kernel/symbol_set.h"
#include "kernel/adf.h"
#include "kernel/argument.h"
#include "kernel/log.h"
#include "kernel/random.h"

namespace vita
{
///
/// Sets up the object.
/// The constructor allocates memory for up to `k_args` argument.
///
symbol_set::symbol_set() : arguments_(gene::k_args), symbols_(), all_("ALL"),
                           by_()
{
  for (unsigned i(0); i < gene::k_args; ++i)
    arguments_[i] = make_unique<argument>(i);

  assert(debug());
}

///
/// Clear the current symbol set.
///
void symbol_set::clear()
{
  *this = {};
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
symbol *symbol_set::get_adt(std::size_t i) const
{
  assert(i < all_.adt.size());
  return all_.adt[i].sym;
}

///
/// \return the number of ADT functions stored.
///
std::size_t symbol_set::adts() const
{
  return all_.adt.size();
}

///
/// \param[in] s symbol to be added.
/// \param[in] wr the weight of `i` (1.0 means standard frequency, 2.0 double
///               probability of selection).
/// \return a raw pointer to the symbol just added (or `nullptr` in case of
///         error).
///
/// Adds a new symbol to the set. We manage to sort the symbols in
/// descending order, with respect to the weight, so the selection algorithm
/// would run faster.
///
symbol *symbol_set::insert(std::unique_ptr<symbol> s, double wr)
{
  assert(s);
  assert(s->debug());
  assert(wr >= 0.0);

  const auto w(static_cast<unsigned>(wr * w_symbol::base_weight));
  const w_symbol ws(s.get(), w);

  symbols_.push_back(std::move(s));

  all_.symbols.insert(ws);

  if (ws.sym->terminal())
  {
    all_.terminals.insert(ws);

    if (ws.sym->auto_defined())
      all_.adt.insert(ws);
  }
  else  // not a terminal
    if (ws.sym->auto_defined())
      all_.adf.insert(ws);

  by_ = by_category(all_);

  return ws.sym;
}

///
void symbol_set::reset_adf_weights()
{
  for (auto adt : all_.adt)
  {
    const auto w(adt.weight);
    const auto delta(w > 1 ? w/2 : w);

    adt.weight -= delta;
    all_.symbols.changed_weight(-delta);
    all_.terminals.changed_weight(-delta);

    if (delta && adt.weight == 0)
    {
      const auto opcode(adt.sym->opcode());
      const auto adt_opcode([opcode](w_symbol s)
                            {
                              return s.sym->opcode() == opcode;
                            });

      erase_if(all_.terminals, adt_opcode);
      erase_if(all_.symbols, adt_opcode);
    }
  }

  for (auto adf : all_.adf)
  {
    const auto w(adf.weight);
    const auto delta(w > 1 ? w/2 : w);

    adf.weight -= delta;
    all_.symbols.changed_weight(-delta);
  }

  by_ = by_category(all_);
}

///
/// \param[in] c a category.
/// \return a random terminal of category `c`.
///
terminal *symbol_set::roulette_terminal(category_t c) const
{
  assert(c < categories());

  return static_cast<terminal *>(by_.category[c].terminals.roulette());
  //return random::element(by_.category[c].terminals);
}

///
/// \param[in] c a category.
/// \return a random symbol of category `c`.
///
symbol *symbol_set::roulette(category_t c) const
{
  assert(c < categories());

  return by_.category[c].symbols.roulette();
}

///
/// \return a random symbol from the set of all symbols.
///
symbol *symbol_set::roulette() const
{
  return all_.symbols.roulette();
}

///
/// \param[in] opcode numerical code used as primary key for a symbol.
/// \return a pointer to the vita::symbol identified by `opcode`
///         (`nullptr` if not found).
///
symbol *symbol_set::decode(opcode_t opcode) const
{
  for (auto s : all_.symbols)
    if (s.sym->opcode() == opcode)
      return s.sym;

  return nullptr;
}

///
/// \param[in] dex the name of a symbol.
/// \return a pointer to the symbol identified by `dex` (0 if not found).
///
/// \attention Please note that opcodes (automatically assigned) are primary
/// keys for symbols. Conversely the name of a symbol is chosen by the
/// user, so, if you don't pay attention, different symbols may have the same
/// name.
///
symbol *symbol_set::decode(const std::string &dex) const
{
  assert(dex != "");

  for (auto s : all_.symbols)
    if (s.sym->display() == dex)
      return s.sym;

  return nullptr;
}

///
/// \return number of categories in the symbol set (>= 1).
///
/// See also data::categories().
///
unsigned symbol_set::categories() const
{
  return static_cast<unsigned>(by_.category.size());
}

///
/// \param[in] c a category.
/// \return number of terminals in category `c`.
///
unsigned symbol_set::terminals(category_t c) const
{
  assert(c < by_.category.size());
  return static_cast<unsigned>(by_.category[c].terminals.size());
}

///
/// \return `true` if there are enough terminals for secure individual
///         generation.
///
/// We want at least one terminal for every category.
///
bool symbol_set::enough_terminals() const
{
  std::set<category_t> need;

  for (const auto &s : all_.symbols)
  {
    const auto arity(s.sym->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
      need.insert(function::cast(s.sym)->arg_category(i));
  }

  for (const auto &i : need)
  {
    const collection &cc(by_.category[i]);

    if (i >= categories() || !cc.terminals.size())
      return false;
  }
  return true;
}

///
/// \param[in] s a symbol
/// \return the weight of `s`.
///
unsigned symbol_set::weight(const symbol *s) const
{
  assert(s);

  for (const auto &i : all_.symbols)
    if (i.sym == s)
      return i.weight;

  return 0;
}

///
/// \param[out] o output stream.
/// \param[in] ss symbol set to be printed.
/// \return output stream including `ss`.
///
/// Useful for debugging purpose.
///
std::ostream &operator<<(std::ostream &o, const symbol_set &ss)
{
  for (const auto &s : ss.all_.symbols)
  {
    o << s.sym->display();

    const auto arity(s.sym->arity());
    if (arity)
      o << '(';
    for (auto j(decltype(arity){0}); j < arity; ++j)
      o << function::cast(s.sym)->arg_category(j)
        << (j + 1 == arity ? "" : ", ");
    if (arity)
      o << ')';

    o << " -> " << s.sym->category() << " (opcode " << s.sym->opcode()
      << ", parametric " << s.sym->parametric()
      << ", weight " << s.weight << ")\n";
  }

  return o << "Sum: " << ss.all_.symbols.sum() << '\n';
}

///
/// \return `true` if the object passes the internal consistency check.
///
bool symbol_set::debug() const
{
  if (!all_.debug())
    return false;

  if (!by_.debug())
    return false;

  if (!enough_terminals())
  {
    print.error("Symbol set doesn't contain enough symbols");
    return false;
  }

  return true;
}

///
/// \brief New empty collection
/// \param[in] n name of the collection
///
symbol_set::collection::collection(std::string n)
  : symbols("symbols"), terminals("terminals"), adf("adf"), adt("adt"),
    name_(std::move(n))
{
}

///
/// \return `true` if the object passes the internal consistency check.
///
bool symbol_set::collection::debug() const
{
  if (!symbols.debug() || !terminals.debug() || !adf.debug() || !adt.debug())
  {
    print.error("(inside ", name_, ")");
    return false;
  }

  for (const auto &s : symbols)
  {
    const bool not_found_t(
      std::find(terminals.begin(), terminals.end(), s) == terminals.end());

    // Terminals must be in the terminals' vector and functions must not be in
    // the terminals' vector.
    if (s.sym->terminal() == not_found_t)
    {
      print.error(name_, ": terminal ", s.sym->display(), " badly stored");
      return false;
    }

    if (s.sym->auto_defined())
    {
      if (s.sym->terminal())
      {
        if (std::find(adt.begin(), adt.end(), s) == adt.end())
        {
          print.error(name_, ": ADT ", s.sym->display(), " badly stored");
          return false;
        }
      }
      else  // function
      {
        if (std::find(adf.begin(), adf.end(), s) == adf.end())
        {
          print.error(name_, ": ADF ", s.sym->display(), " badly stored");
          return false;
        }
      }
    }
  }

  const auto ssize(symbols.size());

  // The following condition should be met at the end of the symbol_set
  // specification.
  // Since we don't want to enforce a particular insertion order (e.g. terminals
  // before functions), we cannot perform the check here.
  //
  //     if (ssize && !terminals.size())
  //     {
  //       print.error(name_, ": no terminal in the symbol set");
  //       return false;
  //     }

  if (ssize < terminals.size())
  {
    print.error(name_, ": wrong terminal set size (more than symbol set)");
    return false;
  }

  if (ssize < adf.size())
  {
    print.error(name_, ": wrong ADF set size (more than symbol set)");
    return false;
  }

  if (ssize < adt.size())
  {
    print.error(name_, ": wrong ADT set size (more than symbol set)");
    return false;
  }

  return true;
}

///
/// \param[in] c a collection containing many categories of symbol.
///
/// Initialize the struct using collection `c` as input parameter (`c` should
/// be a collection containing more than one category).
///
symbol_set::by_category::by_category(const collection &c) : category()
{
  for (auto s : c.symbols)
  {
    const category_t cat(s.sym->category());
    if (cat >= category.size())
    {
      category.resize(cat + 1);
      category[cat] = collection("By category " + std::to_string(cat));
    }

    category[cat].symbols.insert(s);
  }

  for (auto t : c.terminals)
    category[t.sym->category()].terminals.insert(t);

  for (auto adf : c.adf)
    category[adf.sym->category()].adf.insert(adf);

  for (auto adt : c.adt)
    category[adt.sym->category()].adt.insert(adt);

  assert(debug());
}

///
/// \return `true` if the object passes the internal consistency check.
///
bool symbol_set::by_category::debug() const
{
  for (const collection &coll : category)
    if (!coll.debug())
      return false;

  return true;
}

void symbol_set::collection::sum_container::insert(const w_symbol &ws)
{
  elems_.push_back(ws);
  sum_ += ws.weight;

  std::sort(begin(), end(),
            [](w_symbol s1, w_symbol s2) { return s1.weight > s2.weight; });
}

///
/// \return a random symbol from the container.
///
/// Probably the fastest way to produce a realization of a random variable
/// X in a computer is to create a big table where each outcome `i` is
/// inserted a number of times proportional to `P(X=i)`.
///
/// Two fast methods are described in "Fast Generation of Discrete Random
/// Variables" (Marsaglia, Tsang, Wang).
/// Also `std::discrete_distribution` seems quite fast.
///
/// Anyway we choose the "roulette algorithm" because it's very simple and
/// allows changing weights dynamically (performance differences can hardly
/// be measured).
///
symbol *symbol_set::collection::sum_container::roulette() const
{
  const auto slot(random::sup(sum()));

  std::size_t i(0);
  for (auto wedge(elems_[i].weight);
       wedge <= slot;
       wedge += elems_[++i].weight)
  {}

  assert(i < elems_.size());
  return elems_[i].sym;

  // The so called roulette-wheel selection via stochastic acceptance:
  //
  // for (;;)
  // {
  //   const symbol *s(random::element(elems_));
  //
  //   if (random::sup(max) < s->weight)
  //     return s;
  // }
  //
  // Internal tests have proved this is slower for Vita.

  // This is a different approach from Eli Bendersky
  // (<http://eli.thegreenplace.net/>):
  //
  //     unsigned total(0);
  //     std::size_t winner(0);
  //
  //     for (std::size_t i(0); i < symbols.size(); ++i)
  //     {
  //       total += elems_[i]->weight;
  //       if (random::sup(total + 1) < elems_[i]->weight)
  //         winner = i;
  //     }
  //     return elems_[winner].sym;
  //
  // The interesting property of this algorithm is that you don't need to
  // know the sum of weights in advance in order to use it. The method is
  // cool, but slower than the standard roulette.
}

///
/// \return `true` if the object passes the internal consistency check
///
bool symbol_set::collection::sum_container::debug() const
{
  unsigned check_sum(0);

  for (const auto &e : elems_)
  {
    if (!e.sym->debug())
    {
      print.error(name_, ": invalid symbol ", e.sym->display());
      return false;
    }

    check_sum += e.weight;

    if (e.weight == 0 && !(e.sym->terminal() || e.sym->auto_defined()))
    {
      print.error(name_, ": null weight for symbol ", e.sym->display());
      return false;
    }
  }

  if (check_sum != sum())
  {
    print.error(name_, ": incorrect cached sum of weights (stored: ", sum(),
                ", correct: ", check_sum, ')');
    return false;
  }

  return true;
}

}  // namespace vita
