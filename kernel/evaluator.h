/**
 *
 *  \file evaluator.h
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(EVALUATOR_H)
#define      EVALUATOR_H

#include "vita.h"
#include "fitness.h"
#include "random.h"

namespace vita
{

  class individual;

  ///
  /// \a evaluator \c class calculates the fitness of an individual (how good
  /// he is). It's an abstract \c class because the fitness is domain dependent 
  /// (symbolic regression, data classification, automation...).
  /// Note: this \c class shouldn't be confused with the \a interpreter 
  /// \c class (that calculates the output of an individual given an input 
  /// vector).
  ///  
  class evaluator
  {
  public:
    virtual fitness_t run(const individual &) = 0;
  };

  ///
  /// \a random_evaluator \c class is used for debug purpose.
  /// NOTE: the fitness is population independent.
  /// 
  class random_evaluator : public evaluator
  {
  public:
    fitness_t run(const individual &) 
    { 
      return random::between<unsigned>(0,16000); 
    };
  };

}  // namespace vita

#endif  // EVALUATOR_H
