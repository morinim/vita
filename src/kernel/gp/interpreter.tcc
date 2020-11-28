/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INTERPRETER_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_INTERPRETER_TCC)
#define      VITA_INTERPRETER_TCC

///
/// \param[in] ind individual whose value we are interested in
/// \param[in] ctx context in which we calculate the output value (used for
///                the evaluation of ADF). It can be empty (`nullptr`)
///
/// \warning
/// The lifetime of `ind` and `ctx` must extend beyond that of the interpreter.
///
template<class T>
interpreter<T>::interpreter(const T *ind, interpreter *ctx)
  : core_interpreter(), prg_(ind), cache_(ind->size(), ind->categories()),
    ip_(ind->best_), context_(ctx)
{
  Expects(ind);
}

///
/// \param[in] ip locus of the genome we are starting evaluation from
/// \return       the output value of `this` individual
///
template<class T>
value_t interpreter<T>::run_locus(const locus &ip)
{
  for (auto &e : cache_)
    e.valid = false;

  ip_ = ip;
  return (*prg_)[ip_].sym->eval(this);
}

///
/// Calls `run_locus()` using the default starting locus.
///
/// \return the output value of `this` individual
///
template<class T>
inline value_t interpreter<T>::run_nvi()
{
  return run_locus(prg_->best_);
}

///
/// \return the output value of the current terminal symbol
///
template<class T>
terminal::param_t interpreter<T>::fetch_param()
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
/// - <http://wikipedia.org/wiki/Referential_transparency_(computer_science)>
/// - <http://wikipedia.org/wiki/Memoization>
///
template<class T>
value_t interpreter<T>::fetch_arg(unsigned i)
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
      const auto ret((*prg_)[ip_].sym->eval(this));
      ip_ = backup;
      return ret;
    });

  const locus l(g.arg_locus(i));
  auto &elem(cache_(l));

  if (!elem.valid)
  {
    elem.value = get_val(l);
    elem.valid = true;
  }
#if !defined(NDEBUG)
  else // Cache not empty... checking if the cached value is right.
  {
    assert(get_val(l) == elem.value);
  }
#endif

  Ensures(elem.valid);
  return elem.value;
}

///
/// \param[in] i i-th argument of the current ADF
/// \return      the value of the i-th argument of the current ADF function
///
template<class T>
value_t interpreter<T>::fetch_adf_arg(unsigned i)
{
#if !defined(NDEBUG)
  assert(context_);
  assert(i < gene::k_args);

  const gene ctx_g(context_->prg_->operator[](context_->ip_));
  assert(!ctx_g.sym->terminal() && ctx_g.sym->auto_defined());
#endif
  return context_->fetch_arg(i);
}

///
/// \param[in] i `i`-th argument of the current function
/// \return      the index referenced by the `i`-th argument of the current
///              function
///
template<class T>
index_t interpreter<T>::fetch_index(unsigned i) const
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
template<class T>
double interpreter<T>::penalty_locus(const locus &ip)
{
  ip_ = ip;
  return (*prg_)[ip_].sym->penalty(this);
}

///
/// Calls penalty_locus() using the default starting locus.
///
/// \return the penalty for `this` individual
///
template<class T>
double interpreter<T>::penalty_nvi()
{
  return penalty_locus(prg_->best_);
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T>
bool interpreter<T>::is_valid_nvi() const
{
  return ip_.index < prg_->size();
}

///
/// A handy short-cut for one-time execution of an individual.
///
/// \param[in] ind individual/program to be run
/// \return        possible output value of the individual
///
template<class T> value_t run(const T &ind)
{
  return interpreter<T>(&ind).run();
}

#endif  // include guard
