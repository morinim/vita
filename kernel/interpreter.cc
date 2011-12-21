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

#include <boost/lexical_cast.hpp>
#include <boost/none.hpp>

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
    : ip_(ind.best_), context_(ctx), ind_(ind), cache_(ind.size())
  {
  }

  ///
  /// \param[in] ip locus of the genome we are starting evaluation from.
  /// \return the output value of \c this \a individual.
  ///
  boost::any interpreter::operator()(unsigned ip)
  {
    for (unsigned i(0); i < cache_.size(); ++i)
      cache_[i] = boost::none;

    ip_ = ip;
    return ind_.code_[ip_].sym->eval(this);
  }

  ///
  /// \return the output value of \c this \a individual.
  ///
  /// Calls operator()(unsigned) using the the locus of the individual
  /// (ind_.best).
  ///
  boost::any interpreter::operator()()
  {
    return operator()(ind_.best_);
  }

  ///
  /// \return the output value of the current terminal symbol.
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
  /// We use a cache to avoid recalculating the same value during the same
  /// \a interpreter execution.
  /// This means that side effects are not evaluated to date: WE ASSUME
  /// REFERENTIAL TRANSPARENCY for all the expressions.
  /// \li
  /// [http://en.wikipedia.org/wiki/Referential_transparency_(computer_science)]
  /// \li
  /// [http://en.wikipedia.org/wiki/Memoization]
  ///
  boost::any interpreter::eval(unsigned i)
  {
    const gene &g(ind_.code_[ip_]);

    assert(i < g.sym->arity());

    const locus_t locus(g.args[i]);

    if (!cache_[locus])
    {
      const unsigned backup(ip_);
      ip_ = locus;
      assert(ip_ > backup);
      const boost::any ret(ind_.code_[ip_].sym->eval(this));
      ip_ = backup;

      cache_[locus] = ret;
    }
#if !defined(NDEBUG)
    else // Cache not empty... checking if the cached value is right.
    {
      const unsigned backup(ip_);
      ip_ = locus;
      assert(ip_ > backup);
      const boost::any ret(ind_.code_[ip_].sym->eval(this));
      ip_ = backup;
      assert(to_string(ret) == to_string(*cache_[locus]));
    }
#endif

    assert(cache_[locus]);
    return *cache_[locus];

    // return ind_.code_[g.args[i]].sym->eval(interpreter(ind_,context_,
    //                                                    g.args[i]));
  }

  ///
  /// \param[in] i i-th argument of the current ADF.
  /// \return the value of the i-th argument of the curren ADF function.
  ///
  boost::any interpreter::eval_adf_arg(unsigned i)
  {
#if !defined(NDEBUG)
    const gene context_g(context_->ind_.code_[context_->ip_]);

    assert(context_ && context_->check() && i < gene::k_args &&
           (!context_g.sym->terminal() && context_g.sym->auto_defined()));
#endif
    return context_->eval(i);
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

  ///
  /// \param[in] a value that should be converted to \c double.
  /// \return the result of the conversion of \a a in a \c double.
  ///
  /// This function is useful for:
  /// \li debugging purpose (otherwise comparison of \c boost::any values is
  ///     complex);
  /// \li symbolic regression and classification task (the value returned by
  ///     the interpeter will be used in a "numeric way").
  ///
  double interpreter::to_double(const boost::any &a)
  {
    return
      a.type() == typeid(double) ? boost::any_cast<double>(a) :
      a.type() == typeid(int) ? static_cast<double>(boost::any_cast<int>(a)) :
      a.type() == typeid(bool) ? static_cast<double>(boost::any_cast<bool>(a)) :
      0.0;
  }

  ///
  /// \param[in] a value that should be converted to \c std::string.
  /// \return the result of the conversion of \a a in a string.
  ///
  /// This function is useful for debugging purpose (otherwise comparison /
  /// printing of \c boost::any values is complex).
  ///
  std::string interpreter::to_string(const boost::any &a)
  {
    return
      a.type() == typeid(double) ?
      boost::lexical_cast<std::string>(boost::any_cast<double>(a)) :
      a.type() == typeid(int) ?
      boost::lexical_cast<std::string>(boost::any_cast<int>(a)) :
      a.type() == typeid(bool) ?
      boost::lexical_cast<std::string>(boost::any_cast<bool>(a)) :
      boost::any_cast<std::string>(a);
  }
}  // Namespace vita
