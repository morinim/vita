/**
 *
 *  \file evolution_operation.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini. All rights reserved.
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
  operation_strategy::operation_strategy(const evolution *const evo)
    : evo_(evo)
  {
  }

  class uniformcross_op : public operation_strategy
  {
  public:
    explicit uniformcross_op(const evolution *const);

    virtual std::vector<individual> run(const std::vector<unsigned> &,
                                        summary *const);
  };

  uniformcross_op::uniformcross_op(const evolution *const evo)
    : operation_strategy(evo)
  {
  }

  ///
  /// \return
  ///
  std::vector<individual> uniformcross_op::run(
    const std::vector<unsigned> &parent,
    summary *const s)
  {
    assert(parent.check() && s);

    const population &pop = evo_->population();
    const unsigned r1(parent[0]), r2(parent[1]);

    std::vector<individual> off(1);
    if (random::boolean(pop.env().p_cross))
    {
      off[0] = pop[r1].uniform_cross(pop[r2]);
      ++s->crossovers;
    }
    else
      off[0] = pop[random::boolean() ? r1 : r2];

    s->mutations += off[0].mutation();

    assert(off[0].check());

    return off;
  }

  operation_factory::operation_factory(const evolution *const evo)
  {
    put(new uniformcross_op(evo));
  }

  operation_factory::~operation_factory()
  {
    delete strategy_[unicross_mutation];
  }

  operation_strategy *operation_factory::get(unsigned s)
  {
    return strategy_[s];
  }

  unsigned operation_factory::put(operation_strategy *const s)
  {
    strategy_.push_back(s);
    return strategy_.size();
  }
}  // namespace vita
