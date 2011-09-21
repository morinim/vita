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

    virtual individual crossover(const individual &, const individual &) const;
    virtual unsigned mutation(individual *const) const;

    summary *stats;
  };

  standard_op::standard_op(const evolution *const evo, summary *const s)
    : operation_strategy(evo, s)
  {
  }

  ///
  /// \param[in] p1 first parent for crossover.
  /// \param[in] p2 second parent for crossover.
  /// \return the result of the crossover operation between \a p1 and \a p2.
  ///
  individual standard_op::crossover(const individual &p1,
                                    const individual &p2) const
  {
    return individual(p1).uniform_cross(p2);
  }

  ///
  /// \param[in] ind individual that should be mutated.
  /// \return number of mutations performed.
  ///
  unsigned standard_op::mutation(individual *const ind) const
  {
    return ind->mutation();
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
    { cross ? crossover(pop[r1], pop[r2]) : pop[random::boolean() ? r1 : r2]};

    if (cross)
      ++stats_->crossovers;

    stats_->mutations += mutation(&off[0]);

    assert(off[0].check());

    return off;
  }

  class cross1_op : public standard_op
  {
  public:
    cross1_op(const evolution *const, summary *const);

    individual crossover(const individual &, const individual &) const;
  };

  cross1_op::cross1_op(const evolution *const evo, summary *const stats)
    : standard_op(evo, stats)
  {
  }

  ///
  /// \param[in] p1 first parent for crossover.
  /// \param[in] p2 second parent for crossover.
  /// \return the result of the crossover operation between \a p1 and \a p2.
  ///
  individual cross1_op::crossover(const individual &p1,
                                  const individual &p2) const
  {
    return p1.cross1(p2);
  }

  operation_factory::operation_factory(const evolution *const evo,
                                       summary *const stats)
  {
    assert(evo);
    assert(stats);

    put(new standard_op(evo, stats));
    put(new cross1_op(evo, stats));
  }

  operation_factory::~operation_factory()
  {
    // Only predefined operation strategies should be deleted. User defined
    // operation aren't under our responsability.
    delete strategy_[unicross_mutation];
    delete strategy_[cross1_mutation];
  }

  operation_strategy &operation_factory::operator[](unsigned s) const
  {
    assert(s < strategy_.size());
    return *strategy_[s];
  }

  unsigned operation_factory::put(operation_strategy *const s)
  {
    assert(s);
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
