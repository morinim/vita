/**
 *
 *  \file interpreter.cc
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

#include "kernel/interpreter.h"
#include "kernel/adf.h"
#include "kernel/individual.h"
#include "kernel/symbol.h"

namespace vita
{
  ///
  /// \param[in] ind individual whose value we are interested in,
  /// \param[in] ctx context in which we calculate the output value (used for
  ///                the evaluation of ADF).
  ///
  interpreter::interpreter(const individual &ind,
                           interpreter *const ctx)
    : ip_(ind.best_), context_(ctx), ind_(ind),
      cache_(ind.size()),
      context_cache_(ctx ? ctx->ind_.size() : 0)
  {
  }

  ///
  /// \return the output value of \c this \a individual.
  ///
  boost::any interpreter::operator()()
  {
    for (unsigned i(0); i < cache_.size(); ++i)
    {
      cache_[i].empty = true;
      cache_[i].value = boost::any();
    }

    // Probably the context_cache_ vector will be deleted (it was introduced
    // before the cache_ vector).
    for (unsigned i(0); i < context_cache_.size(); ++i)
      context_cache_[i] = boost::any();

    ip_ = ind_.best_;
    return ind_.code_[ip_].sym->eval(this);
  }

  ///
  /// \return the output value of the current terminal  symbol.
  ///
  boost::any interpreter::eval()
  {
    const gene &g(ind_.code_[ip_]);

    assert(g.sym->parametric());
    return g.par;
  }

  ///
  /// \param[in] i i-th argument of the current function.
  /// \return the value of the i-th argument of the current function.
  ///
  boost::any interpreter::eval(unsigned i)
  {
    const gene &g(ind_.code_[ip_]);

    assert(i < g.sym->arity());

    const locus_t locus(g.args[i]);
    if (cache_[locus].empty)
    {
      const unsigned backup(ip_);
      ip_ = locus;
      assert(ip_ > backup);
      const boost::any ret(ind_.code_[ip_].sym->eval(this));
      ip_ = backup;

      cache_[locus].empty = false;
      cache_[locus].value = ret;
    }
#if !defined(NDEBUG)
    else
    {
      const unsigned backup(ip_);
      ip_ = locus;
      assert(ip_ > backup);
      const boost::any ret(ind_.code_[ip_].sym->eval(this));
      ip_ = backup;
      if (ret.type() == typeid(int))
        assert(boost::any_cast<int>(ret) ==
               boost::any_cast<int>(cache_[locus].value));
      else if (ret.type() == typeid(double))
        assert(boost::any_cast<double>(ret) ==
               boost::any_cast<double>(cache_[locus].value));
    }
#endif

    assert(!cache_[locus].empty);
    return cache_[locus].value;

    // return ind_.code_[g.args[i]].sym->eval(interpreter(ind_,context_,
    //                                                    g.args[i]));
  }

  ///
  /// \param[in] i-th argument of the current ADF,
  /// \return the value of the i-th argument of the curren ADF function.
  ///
  boost::any interpreter::eval_adf_arg(unsigned i)
  {
    const gene context_g(context_->ind_.code_[context_->ip_]);

    assert(context_ && context_->check() && i < gene_args &&
           dynamic_cast<const adf_n *>(context_g.sym));

    if (context_cache_[context_g.args[i]].empty())
      context_cache_[context_g.args[i]] = context_->eval(i);

    return context_cache_[context_g.args[i]];
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool interpreter::check() const
  {
    return
      ip_ < ind_.code_.size() &&
      (!context_ || context_->check());
  }
}  // Namespace vita
