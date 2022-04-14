/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/symbol_set.h"
#include "kernel/log.h"
#include "kernel/random.h"
#include "kernel/gp/gene.h"

namespace vita
{
///
/// Sets up the object.
///
/// The constructor allocates memory for up to `k_args` arguments.
///
symbol_set::symbol_set() : symbols_(), views_()
{
  Ensures(is_valid());
}

///
/// Clears the current symbol set.
///
void symbol_set::clear()
{
  //*this = {};

  symbols_.clear();
  views_.clear();
}

///
/// Adds a new symbol to the set.
///
/// \param[in] s  symbol to be added
/// \param[in] wr the weight of `s` (`1.0` means standard frequency, `2.0`
///               double probability of selection)
/// \return       a raw pointer to the symbol just added (or `nullptr` in case
///               of error)
///
/// A symbol with undefined category will be changed to the first free
/// category.
///
symbol *symbol_set::insert(std::unique_ptr<symbol> s, double wr)
{
  Expects(s);
  Expects(wr >= 0.0);

  const auto w(static_cast<weight_t>(wr * w_symbol::base_weight));
  const w_symbol ws(s.get(), w);

  category_t category(s->category());
  if (category == undefined_category)
  {
    category = views_.size();
    s->category(category);
  }

  for (category_t i(views_.size()); i <= category; ++i)
    views_.emplace_back("Collection " + std::to_string(i));
  assert(category < views_.size());

  views_[category].all.insert(ws);

  if (s->terminal())
    views_[category].terminals.insert(ws);
  else  // function
    views_[category].functions.insert(ws);

  symbols_.push_back(std::move(s));
  return ws.sym;
}

template<class F>
void symbol_set::collection::sum_container::scale_weights(double ratio, F f)
{
  for (auto &s : elems_)
    if (f(s))
    {
      sum_ -= s.weight;
      s.weight = static_cast<weight_t>(s.weight * ratio);
      sum_ += s.weight;
    }
}

///
/// \param[in] c a category
/// \return      a random function of category `c`
///
const function &symbol_set::roulette_function(category_t c) const
{
  Expects(c < categories());
  Expects(views_[c].functions.size());

  return static_cast<const function &>(views_[c].functions.roulette());
}

///
/// \param[in] c a category
/// \return      a random terminal of category `c`
///
const terminal &symbol_set::roulette_terminal(category_t c) const
{
  Expects(c < categories());
  Expects(views_[c].terminals.size());

  return static_cast<const terminal &>(views_[c].terminals.roulette());
}

///
/// Extracts a random symbol from the symbol set without bias between terminals
/// and functions .
///
/// \param[in] c a category
/// \return      a random symbol of category `c`
///
/// \attention
/// - \f$P(terminal) = P(function) = 1/2\f$
/// - \f$P(terminal_i|terminal) = \frac{w_i}{\sum_{t \in terminals} w_t}\f$
/// - \f$P(function_i|function) = \frac{w_i}{\sum_{f \in functions} w_f}\f$
///
/// \note
/// If all symbols have the same probability to appear into a chromosome, there
/// could be some problems.
/// For instance, if our problem has many variables (let's say 100) and the
/// function set has only 4 symbols we cannot get too complex trees because the
/// functions have a reduced chance to appear in the chromosome (e.g. it
/// happens in the Forex example).
///
/// \see https://github.com/morinim/vita/wiki/bibliography#1
///
const symbol &symbol_set::roulette(category_t c) const
{
  Expects(c < categories());
  Expects(views_[c].terminals.size());

  if (random::boolean() && views_[c].functions.size())
    return views_[c].functions.roulette();

  return views_[c].terminals.roulette();
}

///
/// Extracts a random symbol from the symbol set.
///
/// \param[in] c a category
/// \return      a random symbol of category `c`
///
/// \attention
/// Given \f$S_t = \sum_{i \in terminals} {w_i}\f$ and
/// \f$S_f = \sum_{i \in functions} {w_i}\f$ we have:
/// - \f$P(terminal_i|terminal) = \frac {w_i} {S_t}\f$
/// - \f$P(function_i|function) = \frac {w_i} {S_f}\f$
/// - \f$P(terminal) = \frac {S_t} {S_t + S_f}\f$
/// - \f$P(function) = \frac {S_f} {S_t + S_f}\f$
///
const symbol &symbol_set::roulette_free(category_t c) const
{
  Expects(c < categories());
  return views_[c].all.roulette();
}

///
/// \param[in] opcode numerical code used as primary key for a symbol
/// \return           a pointer to the vita::symbol identified by `opcode`
///                   (`nullptr` if not found).
///
symbol *symbol_set::decode(opcode_t opcode) const
{
  for (const auto &s : symbols_)
    if (s->opcode() == opcode)
      return s.get();

  return nullptr;
}

///
/// \param[in] dex the name of a symbol
/// \return        a pointer to the symbol identified by `dex` (0 if not found)
///
/// \attention
/// Please note that opcodes are automatically generated and fully identify
/// a symbol (they're primary keys). Conversely the name of a symbol is chosen
/// by the user, so, if you don't pay attention, different symbols may have the
/// same name.
///
symbol *symbol_set::decode(const std::string &dex) const
{
  Expects(!dex.empty());

  for (const auto &s : symbols_)
    if (s->name() == dex)
      return s.get();

  return nullptr;
}

///
/// \return number of categories in the symbol set (`>= 1`)
///
/// \see category_set::size().
///
category_t symbol_set::categories() const
{
  return static_cast<category_t>(views_.size());
}

///
/// \param[in] c a category
/// \return      number of terminals in category `c`
///
std::size_t symbol_set::terminals(category_t c) const
{
  Expects(c < categories());
  return views_[c].terminals.size();
}

///
/// We want at least one terminal for every used category.
///
/// \return `true` if there are enough terminals for secure individual
///         generation
///
bool symbol_set::enough_terminals() const
{
  if (views_.empty())
    return true;

  std::set<category_t> need;

  for (const auto &s : symbols_)
  {
    const auto arity(s->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
      need.insert(function::cast(s.get())->arg_category(i));
  }

  for (const auto &i : need)
    if (i >= categories() || !views_[i].terminals.size())
      return false;

  return true;
}

///
/// \param[in] s a symbol
/// \return      the weight of `s`
///
symbol_set::weight_t symbol_set::weight(const symbol &s) const
{
  for (const auto &ws : views_[s.category()].all)
    if (ws.sym == &s)
      return ws.weight;

  return 0;
}

///
/// Prints the symbol set to an output stream.
///
/// \param[out] o output stream
/// \param[in] ss symbol set to be printed
/// \return       output stream including `ss`
///
/// \note Useful for debugging purpose.
///
std::ostream &operator<<(std::ostream &o, const symbol_set &ss)
{
  for (const auto &s : ss.symbols_)
  {
    o << s->name();

    auto arity(s->arity());
    if (arity)
    {
      o << '(';
      for (decltype(arity) j(0); j < arity; ++j)
        o << function::cast(s.get())->arg_category(j)
          << (j + 1 == arity ? "" : ", ");
      o << ')';
    }

    o << " -> " << s->category() << " (opcode " << s->opcode()
      << ", parametric "
      << (s->terminal() && terminal::cast(s.get())->parametric())
      << ", weight "
      << ss.weight(*s) << ")\n";
  }

  return o;
}

///
/// \return `true` if the object passes the internal consistency check
///
bool symbol_set::is_valid() const
{
  if (!enough_terminals())
  {
    vitaERROR << "Symbol set doesn't contain enough symbols";
    return false;
  }

  return true;
}

///
/// New empty collection.
//
/// \param[in] n name of the collection
///
symbol_set::collection::collection(std::string n)
  : all("all"), functions("functions"), terminals("terminals"),
    name_(std::move(n))
{
}

///
/// \return `true` if the object passes the internal consistency check
///
bool symbol_set::collection::is_valid() const
{
  if (!all.is_valid() || !functions.is_valid() || !terminals.is_valid())
  {
    vitaERROR << "(inside " << name_ << ")";
    return false;
  }

  if (std::any_of(functions.begin(), functions.end(),
                  [](const auto &s) { return s.sym->terminal(); }))
    return false;

  if (std::any_of(terminals.begin(), terminals.end(),
                  [](const auto &s) { return !s.sym->terminal(); }))
    return false;

  for (const auto &s : all)
  {
    if (s.sym->terminal())
    {
      if (std::find(terminals.begin(), terminals.end(), s) == terminals.end())
      {
        vitaERROR << name_ << ": terminal " << s.sym->name()
                  << " badly stored";
        return false;
      }
    }
    else  // function
    {
      if (std::find(functions.begin(), functions.end(), s) == functions.end())
      {
        vitaERROR << name_ << ": function " << s.sym->name()
                  << " badly stored";
        return false;
      }
    }
  }

  const auto ssize(all.size());

  // The following condition should be met at the end of the symbol_set
  // specification.
  // Since we don't want to enforce a particular insertion order (i.e.
  // terminals before functions), we cannot perform the check here.
  //
  //     if (ssize && !terminals.size())
  //     {
  //       vitaERROR << name_ << ": no terminal in the symbol set";
  //       return false;
  //     }

  if (ssize < functions.size())
  {
    vitaERROR << name_ << ": wrong terminal set size (more than symbol set)";
    return false;
  }

  if (ssize < terminals.size())
  {
    vitaERROR << name_ << ": wrong terminal set size (more than symbol set)";
    return false;
  }

  return ssize == functions.size() + terminals.size();
}

///
/// Inserts a weighted symbol in the container.
///
/// \param[in] ws a weighted symbol
///
/// We manage to sort the symbols in descending order, with respect to the
/// weight, so the selection algorithm would run faster.
///
void symbol_set::collection::sum_container::insert(const w_symbol &ws)
{
  elems_.push_back(ws);
  sum_ += ws.weight;

  std::sort(begin(), end(),
            [](auto s1, auto s2) { return s1.weight > s2.weight; });
}

///
/// Extracts a random symbol from the collection.
///
/// \return a random symbol
///
/// Probably the fastest way to produce a realization of a random variable
/// X in a computer is to create a big table where each outcome `i` is
/// inserted a number of times proportional to `P(X=i)`.
///
/// Two fast methods are described in "Fast Generation of Discrete Random
/// Variables" (Marsaglia, Tsang, Wang).
/// Also `std::discrete_distribution` seems quite fast.
///
/// Anyway we choose the "roulette algorithm" because it's very simple.
///
const symbol &symbol_set::collection::sum_container::roulette() const
{
  Expects(sum());

  const auto slot(random::sup(sum()));

  std::size_t i(0);
  for (auto wedge(elems_[i].weight);
       wedge <= slot;
       wedge += elems_[++i].weight)
  {}

  assert(i < elems_.size());
  return *elems_[i].sym;

  // The so called roulette-wheel selection via stochastic acceptance:
  //
  // for (;;)
  // {
  //   const symbol *s(random::element(elems));
  //
  //   if (random::sup(max) < s->weight)
  //     return *s;
  // }
  //
  // Internal tests have proved this is slower for Vita.

  // This is a different approach from Eli Bendersky
  // (<http://eli.thegreenplace.net/>):
  //
  //     weight_t total(0);
  //     std::size_t winner(0);
  //
  //     for (std::size_t i(0); i < size(); ++i)
  //     {
  //       total += elems[i].weight;
  //       if (random::sup(total + 1) < elems[i].weight)
  //         winner = i;
  //     }
  //     return *elems[winner].sym;
  //
  // The interesting property of this algorithm is that you don't need to
  // know the sum of weights in advance in order to use it. The method is
  // cool, but slower than the standard roulette.
}

///
/// \return `true` if the object passes the internal consistency check
///
bool symbol_set::collection::sum_container::is_valid() const
{
  weight_t check_sum(0);

  for (const auto &e : elems_)
  {
    check_sum += e.weight;

    if (e.weight == 0 && !e.sym->terminal())
    {
      vitaERROR << name_ << ": null weight for symbol " << e.sym->name();
      return false;
    }
  }

  if (check_sum != sum())
  {
    vitaERROR << name_ << ": incorrect cached sum of weights (stored: "
              << sum() << ", correct: " << check_sum << ')';
    return false;
  }

  return true;
}

}  // namespace vita
