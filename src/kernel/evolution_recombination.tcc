/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_RECOMBINATION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_RECOMBINATION_TCC)
#define      VITA_EVOLUTION_RECOMBINATION_TCC

///
/// \param[in] pop the current population.
/// \param[in] eva the current evaluator.
/// \param[in] stats pointer to the current set of statistics.
///
template<class T>
strategy<T>::strategy(const population<T> &pop, evaluator<T> &eva,
                      summary<T> *const stats)
  : pop_(pop), eva_(eva), stats_(stats)
{
  assert(stats);
}

///
/// \param[in] parent a vector of ordered parents.
/// \return the offspring.
///
/// This is a quite standard crossover + mutation operator.
///
template<class T>
typename strategy<T>::offspring_t base<T>::run(
  const typename strategy<T>::parents_t &parent)
{
  const auto &pop(this->pop_);
  const auto &env(pop.env());

  assert(0.0 <= env.p_cross);
  assert(env.p_cross <= 1.0);
  assert(0.0 <= env.p_mutation);
  assert(env.p_mutation <= 1.0);
  assert(env.brood_recombination);
  assert(parent.size() >= 2);

  const auto r1(parent[0]), r2(parent[1]);

  if (vita::random::boolean(env.p_cross))
  {
    T off(pop[r1].crossover(pop[r2]));
    ++this->stats_->crossovers;

    // This could be an original contribution of Vita... but it's hard
    // to be sure.
    // It remembers of the hereditary repulsion constraint (I guess you could
    // call it signature repulsion) and seems to:
    // * maintain diversity during the exploration phase;
    // * optimize the exploitation phase.
    while (pop[r1].signature() == off.signature() ||
           pop[r2].signature() == off.signature())
      this->stats_->mutations += off.mutation(env.p_mutation, *env.sset);

    //if (eva_.seen(off))
    //  stats_->mutations += off.mutation(env.p_mutation, *env.sset);

    if (*env.brood_recombination > 0)
    {
      fitness_t fit_off(this->eva_.fast(off));

      unsigned i(0);
      do
      {
        T tmp(pop[r1].crossover(pop[r2]));

        while (pop[r1].signature() == tmp.signature() ||
               pop[r2].signature() == tmp.signature())
          this->stats_->mutations += tmp.mutation(env.p_mutation, *env.sset);

        const auto fit_tmp(this->eva_.fast(tmp));
        if (fit_tmp > fit_off)
        {
          off     =     tmp;
          fit_off = fit_tmp;
        }
      } while (++i < *env.brood_recombination);

      this->stats_->crossovers += i;
    }

    assert(off.debug());
    return {off};
  }
  else // !crossover
  {
    T off(pop[random::boolean() ? r1 : r2]);
    this->stats_->mutations += off.mutation(env.p_mutation, *env.sset);

    assert(off.debug());
    return {off};
  }
}

///
/// \param[in] parent a vector of ordered parents.
/// \return the offspring.
///
/// This is strictly based on the DE crossover operator.
///
template<class T>
typename strategy<T>::offspring_t de<T>::run(
  const typename strategy<T>::parents_t &parent)
{
  assert(parent.size() >= 2);

  const auto &pop(this->pop_);
  const auto &env(pop.env());

  assert(0.0 < env.p_cross);
  assert(env.p_cross <= 1.0);

  const auto a(pickup(pop, parent[0]));
  const auto b(pickup(pop, parent[0]));

  return {pop[parent[0]].crossover(env.p_cross, env.de.weight,
                                   pop[parent[1]], pop[a], pop[b])};
}
#endif  // Include guard
