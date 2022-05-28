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

#include "kernel/gp/mep/interpreter.h"
#include "kernel/gp/mep/i_mep.h"

namespace vita
{

///
/// \param[in] ind individual whose value we are interested in
///
/// \warning
/// The lifetime of `ind` and `ctx` must extend beyond that of the interpreter.
///
interpreter<i_mep>::interpreter(const i_mep *ind)
  : core_interpreter(), prg_(ind), cache_(ind->size(), ind->categories()),
    ip_(ind->best_)
{
  Expects(ind);
}

///
/// \param[in] ip locus of the genome we are starting evaluation from
/// \return       the output value of `this` individual
///
value_t interpreter<i_mep>::run_locus(const locus &ip)
{
  for (auto &e : cache_)
    e.valid = false;

  ip_ = ip;
  return (*prg_)[ip_].sym->eval(*this);
}

///
/// Calls `run_locus()` using the default starting locus.
///
/// \return the output value of `this` individual
///
value_t interpreter<i_mep>::run_nvi()
{
  return run_locus(prg_->best_);
}

///
/// \return the output value of the current terminal symbol
///
terminal_param_t interpreter<i_mep>::fetch_param() const
{
  const gene &g((*prg_)[ip_]);

  assert(g.sym->terminal() && terminal::cast(g.sym)->parametric());
  return g.par;
}

///
/// Fetches the value of the `i`-th argument of the current gene.
///
/// \param[in] i i-th argument of the current gene
/// \return      the required value
///
/// We use a cache to avoid recalculating the same value during the same
/// interpreter execution.
/// This means that side effects are not evaluated to date: WE ASSUME
/// REFERENTIAL TRANSPARENCY for all the expressions.
///
/// \see
/// - <https://en.wikipedia.org/wiki/Referential_transparency>
/// - <https://en.wikipedia.org/wiki/Memoization>
///
value_t interpreter<i_mep>::fetch_arg(unsigned i)
{
  const gene &g((*prg_)[ip_]);

  assert(g.sym->arity());
  assert(i < g.sym->arity());

  const auto get_val(
    [this](const locus &l)
    {
      const locus backup(ip_);
      ip_ = l;
      assert(ip_.index > backup.index);
      const auto ret((*prg_)[ip_].sym->eval(*this));
      ip_ = backup;
      return ret;
    });

  const locus l(g.locus_of_argument(i));
  auto &elem(cache_(l));

  if (!elem.valid)
  {
    elem.value = get_val(l);
    elem.valid = true;
  }
#if !defined(NDEBUG)
  else  // cache not empty... checking if the cached value is right
  {
    assert(get_val(l) == elem.value);
  }
#endif

  Ensures(elem.valid);
  return elem.value;
}

///
/// \param[in] i `i`-th argument of the current function
/// \return      the index referenced by the `i`-th argument of the current
///              function
///
index_t interpreter<i_mep>::fetch_index(unsigned i) const
{
  const gene &g((*prg_)[ip_]);

  assert(g.sym->arity());
  assert(i < g.sym->arity());

  return g.args[i];
}

///
/// \param[in] ip locus of the genome we are starting evaluation from
/// \return       the penalty value for `this` individual
///
double interpreter<i_mep>::penalty_locus(const locus &ip)
{
  ip_ = ip;
  return (*prg_)[ip_].sym->penalty(this);
}

///
/// Calls penalty_locus() using the default starting locus.
///
/// \return the penalty for `this` individual
///
double interpreter<i_mep>::penalty_nvi()
{
  return penalty_locus(prg_->best_);
}

///
/// \return `true` if the object passes the internal consistency check
///
bool interpreter<i_mep>::is_valid_nvi() const
{
  return ip_.index < prg_->size();
}

}  // namespace vita
