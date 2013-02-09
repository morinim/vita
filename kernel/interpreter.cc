/**
 *
 *  \file interpreter.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <boost/none.hpp>

#include "interpreter.h"
#include "adf.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{
  ///
  /// \param[in] ind individual whose value we are interested in,
  /// \param[in] ctx context in which we calculate the output value (used for
  ///                the evaluation of ADF).
  ///
  interpreter::interpreter(const individual &ind, interpreter *const ctx)
    : ip_(ind.best_), context_(ctx), ind_(ind),
      cache_(ind.size(), ind.env_->sset.categories())
  {
  }

  ///
  /// \param[in] ip locus of the genome we are starting evaluation from.
  /// \return the output value of \c this \a individual.
  ///
  any interpreter::run(const locus &ip)
  {
    cache_.fill(boost::none);

    ip_ = ip;
    return ind_[ip_].sym->eval(this);
  }

  ///
  /// \return the output value of \c this \a individual.
  ///
  /// Calls run()(locus) using the the locus of the individual (\c ind_.best).
  ///
  any interpreter::run()
  {
    return run(ind_.best_);
  }

  ///
  /// \return the output value of the current terminal symbol.
  ///
  any interpreter::eval()
  {
    const gene &g(ind_[ip_]);

    assert(g.sym->parametric());
    return any(g.par);
  }

  ///
  /// \param[in] i i-th argument of the current function.
  /// \return the value of the i-th argument of the current function.
  ///
  /// We use a cache to avoid recalculating the same value during the same
  /// \a interpreter execution.
  /// This means that side effects are not evaluated to date: WE ASSUME
  /// REFERENTIAL TRANSPARENCY for all the expressions.
  ///
  /// \see
  /// * <http://en.wikipedia.org/wiki/Referential_transparency_(computer_science)>
  /// * <http://en.wikipedia.org/wiki/Memoization>
  ///
  any interpreter::eval(size_t i)
  {
    const gene &g(ind_[ip_]);

    assert(g.sym->arity());
    assert(i < g.sym->arity());

    const function *const f(function::cast(g.sym));

    const locus l{{g.args[i], f->arg_category(i)}};

    if (!cache_(l))
    {
      const locus backup(ip_);
      ip_ = l;
      assert(ip_[0] > backup[0]);
      const any ret(ind_[ip_].sym->eval(this));
      ip_ = backup;

      cache_(l) = ret;
    }
#if !defined(NDEBUG)
    else // Cache not empty... checking if the cached value is right.
    {
      const locus backup(ip_);
      ip_ = l;
      assert(ip_[0] > backup[0]);
      const any ret(ind_[ip_].sym->eval(this));
      ip_ = backup;
      assert(to_string(ret) == to_string(*cache_(l)));
    }
#endif

    assert(cache_(l));
    return *cache_(l);
  }

  ///
  /// \param[in] i i-th argument of the current ADF.
  /// \return the value of the i-th argument of the curren ADF function.
  ///
  any interpreter::eval_adf_arg(size_t i)
  {
#if !defined(NDEBUG)
    const gene context_g(context_->ind_[context_->ip_]);

    assert(context_ && context_->debug() && i < gene::k_args &&
           (!context_g.sym->terminal() && context_g.sym->auto_defined()));
#endif
    return context_->eval(i);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool interpreter::debug() const
  {
    return ip_[0] < ind_.size() && (!context_ || context_->debug());
  }

  ///
  /// \param[in] a value that should be converted to \c double.
  /// \return the result of the conversion of \a a in a \c double.
  ///
  /// This function is useful for:
  /// \li debugging purpose (otherwise comparison of \c any values is
  ///     complex);
  /// \li symbolic regression and classification task (the value returned by
  ///     the interpeter will be used in a "numeric way").
  ///
  double interpreter::to_double(const any &a)
  {
    return
      a.type() == typeid(double) ? any_cast<double>(a) :
      a.type() == typeid(int) ? static_cast<double>(any_cast<int>(a)) :
      a.type() == typeid(bool) ? static_cast<double>(any_cast<bool>(a)) :
      0.0;
  }

  ///
  /// \param[in] a value that should be converted to \c std::string.
  /// \return the result of the conversion of \a a in a string.
  ///
  /// This function is useful for debugging purpose (otherwise comparison /
  /// printing of \c any values is complex).
  ///
  std::string interpreter::to_string(const any &a)
  {
    return
      a.type() == typeid(double) ?
      boost::lexical_cast<std::string>(any_cast<double>(a)) :
      a.type() == typeid(int) ?
      boost::lexical_cast<std::string>(any_cast<int>(a)) :
      a.type() == typeid(bool) ?
      boost::lexical_cast<std::string>(any_cast<bool>(a)) :
      any_cast<std::string>(a);
  }
}  // Namespace vita
