/**
 *
 *  \file search.h
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
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

  class evolution;

  class search
  {
  public:
    search(problem &);

    void arl(const individual &, evolution &);

    individual run(bool=true, unsigned=1);

    void log() const;

    bool check() const;

  private:
    problem &_prob;
  };
    
}  // namespace vita

#endif  // SEARCH_H
