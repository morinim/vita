/**
 *
 *  \file evolution_replacement.cc
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

  replacement_strategy::replacement_strategy(evolution *const evo)
    : _evo(evo)
  {    
  }

  class tournament_elitist_rp : public replacement_strategy
  {
  public:
    explicit tournament_elitist_rp(evolution *const);

    virtual void run(const std::vector<unsigned> &,
                     const std::vector<individual> &,
                     summary *const);

  protected:
    unsigned tournament(unsigned) const;
  };

  tournament_elitist_rp::tournament_elitist_rp(evolution *const evo)
    : replacement_strategy(evo)
  {    
  }

  unsigned
  tournament_elitist_rp::tournament(unsigned target) const
  {
    const population &pop = _evo->population();

    const unsigned n(pop.size());
    const unsigned mate_zone(pop.env().mate_zone);
    const unsigned rounds(pop.env().rep_tournament);

    unsigned sel(random::ring(target,mate_zone,n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const unsigned j(random::ring(target,mate_zone,n));

      const fitness_t fit_j(_evo->fitness(pop[j]));
      const fitness_t fit_sel(_evo->fitness(pop[sel]));
      if (fit_j < fit_sel)
        sel = j;
    }

    return sel;
  }

  ///
  void
  tournament_elitist_rp::run(const std::vector<unsigned> &parent,
                             const std::vector<individual> &offspring,
                             summary *const s)
  {
    population &pop = _evo->population();

    const fitness_t f_off(_evo->fitness(offspring[0]));

    const unsigned rep_idx(tournament(parent[0]));
    const fitness_t f_rep_idx(_evo->fitness(pop[rep_idx]));
    const bool replace(f_rep_idx < f_off);

    if (replace)
      pop[rep_idx] = offspring[0];

    if (f_off - s->f_best > float_epsilon)
    {
      s->last_imp = s->gen;
      s->best = offspring[0];
      s->f_best = f_off;
    }
  }

  replacement_factory::replacement_factory(evolution *const evo)
  {
    put(new tournament_elitist_rp(evo));
  }

  replacement_factory::~replacement_factory()
  {
    delete _strategy[tournament_elitist];
  }

  replacement_strategy *
  replacement_factory::get(unsigned s)
  {
    return _strategy[s];
  }

  unsigned
  replacement_factory::put(replacement_strategy *const s)
  {
    _strategy.push_back(s);
    return _strategy.size();
  }

}  // namespace vita
