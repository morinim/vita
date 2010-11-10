/**
 *
 *  \file ttable.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/10
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(TTABLE_H)
#define      TTABLE_H

#include "fitness.h"

#include <vector>

namespace vita
{

  class individual;

  class ttable
  {
  public:
    typedef boost::uint64_t hash_t;

    explicit ttable(unsigned);
    ~ttable();

    void clear();

    void insert(const individual &, fitness_t);

    bool find(const individual &, fitness_t *const) const;

    unsigned long long probes() const { return _probes; };
    unsigned long long hits() const { return _hits; };

    bool check() const;

  private:
    static hash_t hash(const std::vector<boost::uint8_t> &);

    struct slot
    {
      hash_t   hash;
      fitness_t fit;
    };

    const hash_t mask;
    slot *const table;

    mutable unsigned long long _probes;
    mutable unsigned long long _hits;
  };
    
}  // namespace vita

#endif  // SYMBOL_H
