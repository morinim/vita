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

#include <boost/none.hpp>

#include "kernel/interpreter.h"
#include "kernel/function.h"
#include "kernel/i_mep.h"

namespace vita
{
  ///
  /// \param[in] ind individual whose value we are interested in,
  /// \param[in] ctx context in which we calculate the output value (used for
  ///                the evaluation of ADF).
  ///
  interpreter<i_mep>::interpreter(const i_mep &ind, interpreter<i_mep> *ctx)
    : core_interpreter(), prg_(ind),
      cache_(ind.size(), ind.sset().categories()), ip_(ind.best_),
      context_(ctx)
  {
  }

  ///
  /// \param[in] ip locus of the genome we are starting evaluation from.
  /// \return the output value of \c this \a individual.
  ///
  any interpreter<i_mep>::run_locus(const locus &ip)
  {
    cache_.fill(boost::none);

    ip_ = ip;
    return prg_[ip_].sym->eval(this);
  }

  ///
  /// \return the output value of \c this \a individual.
  ///
  /// Calls run()(locus) using the the locus of the individual (\c prg_.best).
  ///
  any interpreter<i_mep>::run_nvi()
  {
    return run_locus(prg_.best_);
  }

  ///
  /// \return the output value of the current terminal symbol.
  ///
  any interpreter<i_mep>::fetch_param()
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
  /// * http://en.wikipedia.org/wiki/Referential_transparency_(computer_science)
  /// * http://en.wikipedia.org/wiki/Memoization
  ///
  any interpreter<i_mep>::fetch_arg(unsigned i)
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
  any interpreter<i_mep>::fetch_adf_arg(unsigned i)
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
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool interpreter<i_mep>::debug_nvi() const
  {
    if (context_ && !context_->debug())
      return false;

    if (!prg_.debug())
      return false;

    return ip_.index < prg_.size();
  }
}  // namespace vita
