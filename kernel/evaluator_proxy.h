/**
 *
 *  \file evaluator_proxy.h
 *
 *  Copyright 2011 EOS di Manlio Morini.
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

    double success_rate(const individual &);
    fitness_t operator()(const individual &);

    boost::uint64_t probes() const;
    boost::uint64_t hits() const;

  private:
    /// Access to the real evaluator.
    evaluator *const eva_;

    /// Transposition table (hash cache).
    ttable cache_;
  };
}  // namespace vita

#endif  // EVALUATOR_PROXY_H
