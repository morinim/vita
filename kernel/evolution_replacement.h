/**
 *
 *  \file evolution_replacement.h
 *
 *  \author Manlio Morini
 *  \date 2011/04/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(EVOLUTION_REPLACEMENT_H)
#define      EVOLUTION_REPLACEMENT_H

#include <vector>

#include "vita.h"
#include "individual.h"

namespace vita
{

  class evolution;
  class summary;
  
  ///
  /// The replacement strategy (random, elitist...) for the \a evolution 
  /// \c class. In the strategy design pattern, this \c class is the strategy 
  /// interface and \a evolution is the context.
  ///
  class replacement_strategy
  {
  public:
    explicit replacement_strategy(evolution *const);

    virtual void run(const std::vector<unsigned> &,
                     const std::vector<individual> &,
                     summary *const) = 0;

  protected:
    evolution *const _evo;
  };

  ///
  /// replacement_factory \c class creates a new strategy for the \a evolution
  /// \c class (the context).
  ///
  class replacement_factory
  {
  public:
    enum replacement {tournament_elitist=0};

    replacement_factory(evolution *const);
    ~replacement_factory();

    replacement_strategy *get(unsigned);
    unsigned put(replacement_strategy *const);
    
  private:
    std::vector<replacement_strategy *> _strategy;
  };

}  // namespace vita

#endif  // EVOLUTION_REPLACEMENT_H
