/**
 *
 *  \file evolution_operation.cc
 *
 *  \author Manlio Morini
 *  \date 2011/04/14
 *
 *  This file is part of VITA
 *
 */
  
#include "environment.h"
#include "evolution.h"

namespace vita
{

  operation_strategy::operation_strategy(const evolution *const evo)
    : _evo(evo)
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
  std::vector<individual>
  uniformcross_op::run(const std::vector<unsigned> &parent,
                       summary *const s)
  {
    const population &pop = _evo->population();
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
    delete _strategy[unicross_mutation];
  }

  operation_strategy *
  operation_factory::get(unsigned s)
  {
    return _strategy[s];
  }

  unsigned
  operation_factory::put(operation_strategy *const s)
  {
    _strategy.push_back(s);
    return _strategy.size();
  }

}  // namespace vita
