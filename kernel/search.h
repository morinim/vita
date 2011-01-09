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
#include "evaluator.h"
#include "individual.h"

namespace vita
{

  class evolution;
  class individual;

  class search
  {
  public:
    search(environment &, evaluator *const);

    void arl(const individual &, evolution &);

    individual run(bool=true, unsigned=1);

    void log() const;

    bool check() const;

  private:
    environment *const _env;
    evaluator *const   _eva;
  };
    
}  // namespace vita

#endif  // SEARCH_H
