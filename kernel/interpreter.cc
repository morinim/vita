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
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \param[in] ind individual whose value we are interested in,
  /// \param[in] ctx context in which we calculate the output value (used for
  ///                the evaluation of ADF).
  ///
  interpreter<individual>::interpreter(const individual &ind,
                                       interpreter<individual> *ctx)
    : core_interpreter(ind, ctx), ip_(ind.best_),
      cache_(ind.size(), ind.sset().categories())
  {
  }

  ///
  /// \param[in] ip locus of the genome we are starting evaluation from.
  /// \return the output value of \c this \a individual.
  ///
  any interpreter<individual>::run_locus(const locus &ip)
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
  any interpreter<individual>::run()
  {
    return run_locus(prg_.best_);
  }

  ///
  /// \return the output value of the current terminal symbol.
  ///
  any interpreter<individual>::fetch_param()
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
  /// \a interpreter execution.
  /// This means that side effects are not evaluated to date: WE ASSUME
  /// REFERENTIAL TRANSPARENCY for all the expressions.
  ///
  /// \see
  /// * http://en.wikipedia.org/wiki/Referential_transparency_(computer_science)
  /// * http://en.wikipedia.org/wiki/Memoization
  ///
  any interpreter<individual>::fetch_arg(unsigned i)
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
  any interpreter<individual>::fetch_adf_arg(unsigned i)
  {
    interpreter<individual> *ctx(static_cast<interpreter<individual> *>(
                                   context_));
#if !defined(NDEBUG)
    assert(ctx);
    assert(ctx->debug());
    assert(i < gene::k_args);

    const gene ctx_g(ctx->prg_[ctx->ip_]);
    assert(!ctx_g.sym->terminal() && ctx_g.sym->auto_defined());
#endif
    return ctx->fetch_arg(i);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool interpreter<individual>::debug() const
  {
    if (!core_interpreter<individual>::debug())
      return false;

    return ip_.index < prg_.size();
  }
}  // namespace vita
