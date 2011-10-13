/**
 *
 *  \file function.cc
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

#include "kernel/function.h"

namespace vita
{
  /// This is the default value for the function constructor. Weights are
  /// used by the symbol_set::roulette method to control the probability of
  /// extraction of the symbols.
  unsigned function::default_weight(100);

  ///
  /// \param[in] dis string representation of the function (e.g. for the plus
  ///                \a function it could by "ADD" or "+").
  /// \param[in] t type of the function (i.e. the type of the output value).
  /// \param[in] args parameters (type and number) of the function.
  /// \param[in] w the weight of the function (used for random initialization).
  /// \param[in] asve \c true if the function is associative (e.g. addition is
  ///                 associative, division isn't).
  ///
  function::function(const std::string &dis, symbol_t t,
                     const std::vector<symbol_t> &args, unsigned w, bool asve)
    : symbol(dis, t, w), associative_(asve)
  {
    assert(args.size() <= gene::k_args);

    for (arity_ = 0; arity_ < args.size(); ++arity_)
      argt_[arity_] = args[arity_];

    for (unsigned i(arity()); i < gene::k_args; ++i)
      argt_[i] = sym_void;

    assert(check());
  }

  ///
  /// \param[in] dis string representation of the function (e.g. for the plus
  ///                \a function it could by "ADD" or "+").
  /// \param[in] t type of the function (i.e. the type of the output value).
  /// \param[in] n number of input parameter of the function.
  /// \param[in] w the weight of the function (used for random initialization).
  /// \param[in] asve \c true if the function is associative (e.g. addition is
  ///                 associative, division isn't).
  ///
  /// This constructor is used to quickly create a function of homogeneous
  /// signature (all parameters are of the same type).
  ///
  function::function(const std::string &dis, symbol_t t,
                     unsigned n, unsigned w, bool asve)
    : symbol(dis, t, w), arity_(n), associative_(asve)
  {
    assert(n <= gene::k_args);

    unsigned i(0);
    for (; i <     arity_; ++i)    argt_[i] =        t;
    for (; i < gene::k_args; ++i)  argt_[i] = sym_void;

    assert(check());
  }

  ///
  /// \return \c true if the \a function passes the internal consistency check.
  ///
  bool function::check() const
  {
    return arity_ && arity_ <= gene::k_args && symbol::check();
  }
}  // Namespace vita
