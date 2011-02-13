/**
 *
 *  \file environment.h
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(ENVIRONMENT_H)
#define      ENVIRONMENT_H

#include <cmath>
#include <iostream>
#include <string>

#include "vita.h"
#include "symbol_set.h"

namespace vita
{

  class environment
  {
  public:
    environment();

    void log(std::ostream & = std::cout) const;

    void insert(symbol *const);

    /// The number of genes (maximum length of an evolved program in the 
    /// population).
    unsigned code_length;

    /// Mutation probability. Mutation is one of the principal "search 
    /// operators" used to transform programs in the Genetic Programming
    /// algorithm. Mutation causes random changes in individuals.
    double p_mutation;
    /// Crossover probability.
    double p_cross;

    /// Population's size (number of programs/individuals in the population).
    unsigned long individuals;

    /// Size of the tournament to choose the parents.
    /// Tournament sizes tend to be small relative to the population size. The
    /// ratio of tournament size to population size can be used as a measure of
    /// selective pressure. Note that a tournament size of 1 would be 
    /// equivalent to selecting individuals at random (within the mate zone).
    unsigned par_tournament;
    /// Size of the tournament to choose replaced individuals.
    /// Tournament sizes tend to be small relative to the population size. The
    /// ratio of tournament size to population size can be used as a measure of
    /// selective pressure. Note that a tournament size of 1 would be 
    /// equivalent to selecting individuals at random (within the mate zone).
    unsigned rep_tournament;
    /// Mating zone. 0 for panmictic.
    unsigned mate_zone;

    /// Maximun number of generations allowed before terminate a run (0 means
    /// no limit).
    unsigned g_since_start;
    /// Stop a run when we cannot see improvements within g_without_improvement
    /// generations.
    unsigned g_without_improvement;

    /// Should we use Adaptive Representation through Learning?
    bool arl;

    /// 2^ttable_size is the number of elements of the transposition table.
    unsigned ttable_size;

    /// Save a list of active ADF?
    bool stat_arl;

    /// Where shuld we save statistics / status files?
    std::string stat_dir;

    /// Save statistics every 'stat_period' generation (0 means don't save).
    unsigned stat_period;

    /// Save the environment details before a run?
    bool stat_env;

    /// Should we save a dynamic execution status file?
    bool stat_dynamic;

    /// Should we save a summary of the run?
    bool stat_summary;

    symbol_set   sset;

    bool check() const;
  };

}  // namespace vita

#endif  // ENVIRONMENT_H
