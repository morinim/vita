/**
 *
 *  \file evolution_operation.h
 *
 *  \author Manlio Morini
 *  \date 2011/04/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(EVOLUTION_OPERATION_H)
#define      EVOLUTION_OPERATION_H

#include <vector>

#include "vita.h"
#include "individual.h"

namespace vita
{

  class evolution;
  class summary;
  
  ///
  /// The operation strategy (crossover, recombination, mutation...) for the
  /// \a evolution \c class. In the strategy design pattern, this \c class is 
  /// the strategy interface and \a evolution is the context.
  ///
  class operation_strategy
  {
  public:
    explicit operation_strategy(const evolution *const);

    virtual std::vector<individual> run(const std::vector<unsigned> &,
                                        summary *const) = 0;

  protected:
    const evolution *const _evo;
  };

  ///
  /// operation_factory \c class creates a new strategy for the \a evolution
  /// \c class (the context).
  ///
  class operation_factory
  {
  public:
    enum operation {unicross_mutation=0};

    operation_factory(const evolution *const);
    ~operation_factory();

    operation_strategy *get(unsigned);
    unsigned put(operation_strategy *const);
    
  private:
    std::vector<operation_strategy *> _strategy;
  };

}  // namespace vita

#endif  // EVOLUTION_OPERATION_H
