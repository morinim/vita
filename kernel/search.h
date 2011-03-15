/**
 *
 *  \file search.h
 *
 *  \author Manlio Morini
 *  \date 2011/03/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(SEARCH_H)
#define      SEARCH_H

#include "vita.h"
#include "individual.h"
#include "problem.h"

namespace vita
{
  
  template <class T> class distribution;
  class evolution;
  class summary;

  class search
  {
  public:
    search(problem &);

    void arl(const individual &, evolution &);

    individual run(bool=true, unsigned=1, fitness_t = -5.0);

    void log(const summary &, const distribution<fitness_t> &, 
             unsigned, unsigned) const;

    bool check() const;

  private:
    problem &_prob;
  };
    
}  // namespace vita

#endif  // SEARCH_H
