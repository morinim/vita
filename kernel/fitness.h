/**
 *
 *  \file fitness.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 */
  
#if !defined(FITNESS_H)
#define      FITNESS_H

#include "vita.h"

namespace vita
{

  typedef double fitness_t;

  inline double distance(fitness_t f1, fitness_t f2) {return std::fabs(f1-f2);}

}  // namespace vita

#endif  // FITNESS_H
