/**
 *
 *  \file evolution_selection.cc
 *
 *  \author Manlio Morini
 *  \date 2011/04/13
 *
 *  This file is part of VITA
 *
 */
  
#include "environment.h"
#include "evolution.h"

namespace vita
{

  selection_strategy::selection_strategy(const evolution *const evo)
    : _evo(evo)
  {    
  }

  class tournament_selection : public selection_strategy
  {
  public:
    explicit tournament_selection(const evolution *const);

    virtual std::vector<unsigned> run();

  protected:
    unsigned tournament(unsigned) const;
  };

  tournament_selection::tournament_selection(const evolution *const evo)
    : selection_strategy(evo)
  {    
  }

  ///
  /// \param[in] target index of an individual in the population.
  /// \return index of the best individual found.
  ///
  /// Tournament selection works by selecting a number of individuals from the 
  /// population at random, a tournament, and then choosing only the best 
  /// of those individuals.
  /// Recall that better individuals have highter fitnesses.
  ///
  unsigned
  tournament_selection::tournament(unsigned target) const
  {
    const unsigned n(_evo->population().size());
    const unsigned mate_zone(_evo->population().env().mate_zone);
    const unsigned rounds(_evo->population().env().par_tournament);

    unsigned sel(random::ring(target,mate_zone,n));
    for (unsigned i(1); i < rounds; ++i)
    {
      const unsigned j(random::ring(target,mate_zone,n));

      const fitness_t fit_j(_evo->fitness(_evo->population()[j]));
      const fitness_t fit_sel(_evo->fitness(_evo->population()[sel]));

      if (fit_j > fit_sel)
        sel = j;
    }

    return sel;
  }

  ///
  /// \return
  ///
  std::vector<unsigned> 
  tournament_selection::run()
  {
    std::vector<unsigned> ret(2);

    ret[0] = tournament(_evo->population().size());
    ret[1] = tournament(ret[0]);

    return ret;
  }

  selection_factory::selection_factory(const evolution *const evo)
  {
    put(new tournament_selection(evo));
  }

  selection_factory::~selection_factory()
  {
    delete _strategy[tournament];
  }

  selection_strategy *
  selection_factory::get(unsigned s)
  {
    return _strategy[s];
  }

  unsigned
  selection_factory::put(selection_strategy *const s)
  {
    _strategy.push_back(s);
    return _strategy.size();
  }

}  // namespace vita
