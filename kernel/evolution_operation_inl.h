/**
 *
 *  \file evolution_operation_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_OPERATION_INL_H)
#define      EVOLUTION_OPERATION_INL_H

///
/// \param[in] evo pointer to the current evolution object.
/// \param[in] stats pointer to the current set of statistics.
///
template<class T>
operation_strategy<T>::operation_strategy(const evolution<T> *const evo,
                                          summary<T> *const stats)
  : evo_(evo), stats_(stats)
{
  assert(evo);
  assert(stats);
}

///
/// \param[in] evo pointer to the current evolution object.
/// \param[in] stats pointer to the current set of statistics.
///
template<class T>
standard_op<T>::standard_op(const evolution<T> *const evo, summary<T> *const s)
  : operation_strategy<T>(evo, s)
{
}

///
/// \param[in] parent a vector of ordered parents.
/// \return the offspring.
///
/// This is a quite standard crossover + mutation operator.
///
template<class T>
std::vector<T> standard_op<T>::run(const std::vector<size_t> &parent)
{
  assert(parent.size() >= 2);

  const population<T> &pop(operation_strategy<T>::evo_->population());
  const environment &env(pop.env());

  assert(env.p_cross);
  assert(env.p_mutation);
  assert(env.brood_recombination);

  const auto r1(parent[0]), r2(parent[1]);

  if (random::boolean(*env.p_cross))
  {
    T off(T::crossover(pop[r1], pop[r2]));
    ++operation_strategy<T>::stats_->crossovers;

    // This could be an original contribution of Vita... but it's hard
    // to be sure.
    // It remembers of the hereditary repulsion constraint (I guess you could
    // call it signature repulsion) and seems to:
    // * maintain diversity during the exploration phase;
    // * optimize the exploitation phase.
    while (pop[r1].signature() == off.signature() ||
           pop[r2].signature() == off.signature())
      operation_strategy<T>::stats_->mutations += off.mutation();

    //if (evo_->seen(off))
    //  stats_->mutations += off.mutation();

    if (*env.brood_recombination > 0)
    {
      fitness_t fit_off(operation_strategy<T>::evo_->fast_fitness(off));

      unsigned i(0);
      do
      {
        T tmp(T::crossover(pop[r1], pop[r2]));

        while (pop[r1].signature() == tmp.signature() ||
               pop[r2].signature() == tmp.signature())
          operation_strategy<T>::stats_->mutations += tmp.mutation();

        const auto fit_tmp(operation_strategy<T>::evo_->fast_fitness(tmp));
        if (fit_tmp > fit_off)
        {
          off     =     tmp;
          fit_off = fit_tmp;
        }
      } while (++i < *env.brood_recombination);

      operation_strategy<T>::stats_->crossovers += i;
    }

    assert(off.debug());
    return {off};
  }
  else // !crossover
  {
    T off(pop[random::boolean() ? r1 : r2]);
    operation_strategy<T>::stats_->mutations += off.mutation();

    assert(off.debug());
    return {off};
  }
}
#endif  // EVOLUTION_OPERATION_INL_H
