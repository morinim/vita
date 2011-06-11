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

    for (argc_ = 0; argc_ < args.size(); ++argc_)
      argt_[argc_] = args[argc_];

    for (unsigned i(argc()); i < gene::k_args; ++i)
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
    : symbol(dis, t, w), argc_(n), associative_(asve)
  {
    assert(n <= gene::k_args);

    unsigned i(0);
    for (; i <     argc_; ++i)     argt_[i] =        t;
    for (; i < gene::k_args; ++i)  argt_[i] = sym_void;

    assert(check());
  }

  ///
  /// \return the number of arguments (0 arguments => terminal).
  ///
  unsigned function::argc() const
  {
    assert(argc_);
    return argc_;
  }

  ///
  /// \param[in] index of a function argument.
  /// \return type of the i-th function argument.
  ///
  symbol_t function::arg_type(unsigned i) const
  {
    assert(i < gene::k_args);
    return argt_[i];
  }

  ///
  /// \return \c true if the function is associative (e.g. sum is associative,
  ///         division isn't).
  ///
  bool function::associative() const
  {
    return associative_;
  }

  ///
  /// \return \c false (function are never parametric).
  ///
  bool function::parametric() const
  {
    return false;
  }

  ///
  /// \return \c true if the \a function passes the internal consistency check.
  ///
  bool function::check() const
  {
    return argc_ && argc_ <= gene::k_args && symbol::check();
  }
}  // Namespace vita
