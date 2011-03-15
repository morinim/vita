/**
 *
 *  \file analyzer.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/10
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(ANALYZER_H)
#define      ANALYZER_H

#include <list>
#include <map>
#include <set>

#include "vita.h"
#include "distribution.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{
  struct stats
  {
    unsigned counter[2];
  };

  class analyzer
  {
  public:
    typedef std::map<const symbol *,stats>::const_iterator const_iterator;

    const_iterator begin() const;
    const_iterator end() const;
    
    analyzer();

    unsigned run(const individual &);
    void add(const individual &, fitness_t);

    void clear();

    boost::uint64_t functions(bool) const;
    boost::uint64_t terminals(bool) const;

    const distribution<fitness_t> &fit_dist() const;
    const distribution<double> &length_dist() const;

    bool check() const;

  private:
    void count(const gene &, bool);

    std::map<const symbol *, stats> _info;

    distribution<fitness_t> _fit;
    distribution<double> _length;

    boost::uint64_t _functions[2];
    boost::uint64_t _terminals[2];
  };

}  // namespace vita

#endif  // ANALYZER_H
