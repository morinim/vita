/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MEP_INTERPRETER_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_INTERPRETER_TCC)
#define      VITA_INTERPRETER_TCC

///
/// \param[in] ind individual whose value we are interested in. The lifetime
///                of \a ind must extend beyond that of the interpreter.
/// \param[in] ctx context in which we calculate the output value (used for
///                the evaluation of ADF). It can be empty (\c nullptr).
///                The lifetime of \a ctx must extend beyon that of the
///                interpreter.
///
template<class T>
interpreter<T>::interpreter(const T *ind, interpreter<T> *ctx)
  : core_interpreter(), prg_(*ind),
    cache_(ind->size(), ind->sset().categories()), ip_(ind->best_),
    context_(ctx)
{
  assert(ind);
}

///
/// \param[in] ip locus of the genome we are starting evaluation from.
/// \return the output value of \c this \a individual.
///
template<class T>
any interpreter<T>::run_locus(const locus &ip)
{
  cache_.fill(boost::none);

  ip_ = ip;
  return prg_[ip_].sym->eval(this);
}

///
/// \return the output value of \c this \a individual.
///
/// Calls run_locus() using the default starting locus.
///
template<class T>
any interpreter<T>::run_nvi()
{
  return run_locus(prg_.best_);
}

///
/// \return the output value of the current terminal symbol.
///
template<class T>
any interpreter<T>::fetch_param()
{
  const gene &g(prg_[ip_]);

  assert(g.sym->parametric());
  return any(g.par);
}

///
/// \param[in] i i-th argument of the current function.
/// \return the value of the i-th argument of the current function.
///
/// We use a cache to avoid recalculating the same value during the same
/// \c interpreter execution.
/// This means that side effects are not evaluated to date: WE ASSUME
/// REFERENTIAL TRANSPARENCY for all the expressions.
///
/// \see
/// * <http://en.wikipedia.org/wiki/Referential_transparency_(computer_science)>
/// * <http://en.wikipedia.org/wiki/Memoization>
///
template<class T>
any interpreter<T>::fetch_arg(unsigned i)
{
  const gene &g(prg_[ip_]);

  assert(g.sym->arity());
  assert(i < g.sym->arity());

  const function &f(*function::cast(g.sym));

  const locus l{g.args[i], f.arg_category(i)};

  if (!cache_(l))
  {
    const locus backup(ip_);
    ip_ = l;
    assert(ip_.index > backup.index);
    cache_(l) = prg_[ip_].sym->eval(this);
    ip_ = backup;
  }
#if !defined(NDEBUG)
  else // Cache not empty... checking if the cached value is right.
  {
    const locus backup(ip_);
    ip_ = l;
    assert(ip_.index > backup.index);
    const any ret(prg_[ip_].sym->eval(this));
    ip_ = backup;
    assert(to<std::string>(ret) == to<std::string>(*cache_(l)));
  }
#endif

  assert(cache_(l));
  return *cache_(l);
}

///
/// \param[in] i i-th argument of the current ADF.
/// \return the value of the i-th argument of the current ADF function.
///
template<class T>
any interpreter<T>::fetch_adf_arg(unsigned i)
{
#if !defined(NDEBUG)
  assert(context_);
  assert(context_->debug());
  assert(i < gene::k_args);

  const gene ctx_g(context_->prg_[context_->ip_]);
  assert(!ctx_g.sym->terminal() && ctx_g.sym->auto_defined());
#endif
  return context_->fetch_arg(i);
}

///
/// \param[in] i i-th argument of the current function.
/// \return the index referenced by the i-th argument of the current function.
///
template<class T>
index_t interpreter<T>::fetch_index(unsigned i) const
{
  const gene &g(prg_[ip_]);

  assert(g.sym->arity());
  assert(i < g.sym->arity());

  return g.args[i];
}

///
/// \param[in] ip locus of the genome we are starting evaluation from.
/// \return the penalty value for \c this \a individual.
///
template<class T>
double interpreter<T>::penalty_locus(const locus &ip)
{
  ip_ = ip;
  return prg_[ip_].sym->penalty(this);
}

///
/// \return the penalty for \c this \a individual.
///
/// Calls penalty_locus() using the default starting locus.
//
template<class T>
double interpreter<T>::penalty_nvi()
{
  return penalty_locus(prg_.best_);
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool interpreter<T>::debug_nvi() const
{
  if (context_ && !context_->debug())
    return false;

  if (!prg_.debug())
    return false;

  return ip_.index < prg_.size();
}
#endif  // Include guard
