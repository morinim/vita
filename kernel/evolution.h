/**
 *
 *  \file evolution.h
 *
 *  \author Manlio Morini
 *  \date 2011/04/11
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
  class evolution;

  struct summary
  {
    summary() { clear(); };

    void clear();

    boost::uint64_t ttable_probes;
    boost::uint64_t   ttable_hits;

    boost::uint64_t  mutations;
    boost::uint64_t crossovers;

    unsigned gen;
    unsigned testset;
    unsigned last_imp;

    analyzer az;

    individual  best;
    fitness_t f_best;
  };

  ///
  /// The selection strategy for the \a evolution class. In a strategy design
  /// pattern, this \c class is the strategy and \a evolution is the context.
  ///
  class selection_strategy
  {
  public:
    explicit selection_strategy(const evolution &evo);

    virtual std::vector<unsigned> run() = 0;

  protected:
    const evolution &_evo;
  };

  class tournament_selection : public selection_strategy
  {
  public:
    explicit tournament_selection(const evolution &evo);

    virtual std::vector<unsigned> run();

  protected:
    unsigned tournament(unsigned) const;
  };


  class evolution
  {
  public:
    evolution(vita::population &, evaluator *const);
    ~evolution();

    const summary &run(bool, selection_strategy *const = 0);

    const vita::population &population() const;
    fitness_t fitness(const individual &) const;

    void pick_stats(analyzer *const);

    bool check() const;

  private:
    void log() const;
    void pick_stats();

    bool stop_condition() const;
    unsigned rep_tournament(unsigned) const;

    vita::population       &_pop;
    evaluator_proxy *const  _eva;
    summary               _stats;
    unsigned          _run_count;
  };

  ///
  /// \example example6.cc
  /// Simple symbolic regression example (\f$x^2+y^2-z^2\f$).
  ///

}  // namespace vita

#endif  // EVOLUTION_H
