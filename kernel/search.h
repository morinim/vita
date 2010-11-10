/**
 *
 *  \file search.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(SEARCH_H)
#define      SEARCH_H

#include "vita.h"
#include "individual.h"

namespace vita
{

  class individual;
  class population;

  class search
  {
  public:
    explicit search(environment &);

    void arl(const individual &, population &);

    individual run(bool=true, unsigned=1);

    void log() const;

    bool check() const;

  private:
    environment *const _env;
  };
    
}  // namespace vita

#endif  // SEARCH_H
