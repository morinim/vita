/**
 *
 *  \file evolution_selection.h
 *
 *  \author Manlio Morini
 *  \date 2011/04/13
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(EVOLUTION_SELECTION_H)
#define      EVOLUTION_SELECTION_H

#include <vector>

#include "vita.h"

namespace vita
{

  class evolution;

  ///
  /// The selection strategy (tournament, fitness proportional...) for the
  /// \a evolution \c class. In the strategy design pattern, this \c class is
  /// the strategy interface and \a evolution is the context.
  ///
  class selection_strategy
  {
  public:
    explicit selection_strategy(const evolution *const);

    virtual std::vector<unsigned> run() = 0;

  protected:
    const evolution *const _evo;
  };

  ///
  /// selection_factory \c class creates a new strategy for the \a evolution
  /// \c class (the context).
  ///
  class selection_factory
  {
  public:
    enum strategy {tournament=0};

    selection_factory(const evolution *const);
    ~selection_factory();

    selection_strategy *get(unsigned);
    unsigned put(selection_strategy *const);

  private:
    std::vector<selection_strategy *> _strategy;
  };

}  // namespace vita

#endif  // EVOLUTION_SELECTION_H
