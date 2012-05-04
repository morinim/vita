/**
 *
 *  \file evaluator_proxy.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVALUATOR_PROXY_H)
#define      EVALUATOR_PROXY_H

#include "kernel/vita.h"
#include "kernel/evaluator.h"
#include "kernel/ttable.h"

namespace vita
{
  ///
  /// Provide a surrogate for an \a evaluator to control access to it. The
  /// reason for controlling access is to cache fitness scores of individuals.
  /// \c evaluator_proxy uses an ad-hoc internal hash table
  /// (\a ttable).
  ///
  class evaluator_proxy : public evaluator
  {
  public:
    evaluator_proxy(evaluator *const, unsigned);

    void clear();

    score_t operator()(const individual &);

    boost::uint64_t probes() const;
    boost::uint64_t hits() const;

  private:
    /// Access to the real evaluator.
    evaluator *const eva_;

    /// Transposition table (hash table cache).
    ttable cache_;
  };
}  // namespace vita

#endif  // EVALUATOR_PROXY_H
