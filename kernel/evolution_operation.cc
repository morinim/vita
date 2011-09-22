/**
 *
 *  \file evolution_operation.cc
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

#include "kernel/evolution_operation.h"
#include "kernel/environment.h"
#include "kernel/evolution.h"

namespace vita
{
  operation_strategy::operation_strategy(const evolution *const evo,
                                         summary *const stats)
    : evo_(evo), stats_(stats)
  {
    assert(evo);
    assert(stats);
  }

  ///
  /// This \c class defines the program skeleton of a standard genetic
  /// programming crossover plus mutation operation. It's a template method
  /// design pattern: one or more of the algorithm steps can be overriden
  /// by subclasses to allow differing behaviors while ensuring that the
  /// iverarching algorithm is still followed.
  ///
  class standard_op : public operation_strategy
  {
  public:
    standard_op(const evolution *const, summary *const);

    std::vector<individual> operator()(const std::vector<unsigned> &);

    summary *stats;
  };

  standard_op::standard_op(const evolution *const evo, summary *const s)
    : operation_strategy(evo, s)
  {
  }

  ///
  /// \return the offspring.
  ///
  /// This is a quite standard crossover + mutation operator.
  ///
  std::vector<individual> standard_op::operator()(
    const std::vector<unsigned> &parent)
  {
    assert(parent.size() >= 2);

    const population &pop = evo_->population();
    const unsigned r1(parent[0]), r2(parent[1]);

    const bool cross(random::boolean(pop.env().p_cross));

    std::vector<individual> off
    { cross ? pop[r1].crossover(pop[r2]) : pop[random::boolean() ? r1 : r2]};

    if (cross)
      ++stats_->crossovers;

    stats_->mutations += off[0].mutation();

    assert(off[0].check());

    return off;
  }

  operation_factory::operation_factory(const evolution *const evo,
                                       summary *const stats)
  {
    assert(evo);
    assert(stats);

    add(new standard_op(evo, stats));
  }

  operation_factory::~operation_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[crossover_mutation];
  }

  operation_strategy &operation_factory::operator[](unsigned s) const
  {
    assert(s < strategy_.size());
    return *strategy_[s];
  }

  unsigned operation_factory::add(operation_strategy *const s)
  {
    assert(s);
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
