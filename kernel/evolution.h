/**
 *
 *  \file evolution.h
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(EVOLUTION_H)
#define      EVOLUTION_H

#include "vita.h"
#include "analyzer.h"
#include "evaluator_proxy.h"
#include "population.h"

namespace vita
{

  class environment;

  struct summary
  {
    summary() { clear(); };

    void clear();

    unsigned long long ttable_probes;
    unsigned long long   ttable_hits;

    unsigned long long  mutations;
    unsigned long long crossovers;

    unsigned gen;
    unsigned testset;
    unsigned last_imp;

    analyzer az;

    individual  best;
    fitness_t f_best;
  };

  class evolution
  {
  public:
    evolution(environment &, evaluator *const);
    ~evolution();

    const summary &run(bool);

    const vita::population &population() const;
    fitness_t fitness(const individual &) const;

    void pick_stats(analyzer *const);

    bool check() const;

  private:
    void log() const;
    void pick_stats();

    bool stop_condition() const;
    unsigned tournament(unsigned, bool) const;

    environment            *_env;
    vita::population *const _pop;
    evaluator_proxy *const  _eva;
    summary               _stats;
    unsigned          _run_count;
  };

}  // namespace vita

#endif  // EVOLUTION_H
