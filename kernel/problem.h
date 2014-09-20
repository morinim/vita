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

#if !defined(VITA_PROBLEM_H)
#define      VITA_PROBLEM_H

#include "kernel/data.h"
#include "kernel/environment.h"
#include "kernel/symbol_set.h"
#include "kernel/vitafwd.h"

namespace vita
{
  ///
  /// \brief The interface of a typical genetic programming problem
  ///
  /// \tparam P a function object (function, lambda, functor) used as penalty
  ///           function for constrained problems.
  ///           For unconstrained problems \a P should be \c std::false_type.
  ///
  /// \note
  /// What a horror! Public data members... please read the coding style
  /// document for project Vita
  /// (<http://code.google.com/p/vita/wiki/CodingStyle>).
  template<class P = std::false_type>
  class problem
  {
  public:
    problem();
    explicit problem(P);

    virtual vita::data *data();

    virtual void clear(bool);

    virtual bool debug(bool) const;

  public:  // Public data members
    environment env;

    symbol_set sset;

    /// A parameterless penalty function as described in "An Efficient
    /// Constraint Handling Method for Genetic Algorithms" - Kalyanmoy Deb.
    ///
    /// Problems without constraints don't need a penalty function, so before
    /// calling \a penalty it's a good idea to check:
    ///
    ///     if (prob.penalty)
    ///       // use penalty(...)
    ///
    /// This works since, for unconstrained problems, \a P is
    /// \c std::false_type.
    P penalty;
  };

  template<class P> problem<P> make_problem(P);

#include "kernel/problem_inl.h"
}  // namespace vita

#endif  // Include guard
