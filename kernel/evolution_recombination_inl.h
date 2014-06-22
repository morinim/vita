/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_RECOMBINATION_INL_H)
#define      VITA_EVOLUTION_RECOMBINATION_INL_H

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
std::vector<T> base<T>::run(const std::vector<coord> &parent)
{
  assert(parent.size() == 2);

  const auto &pop(this->pop_);
  const auto &env(pop.env());

  assert(env.p_cross >= 0.0);
  assert(env.p_mutation >= 0.0);
  assert(env.brood_recombination);

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
      this->stats_->mutations += off.mutation();

    //if (eva_.seen(off))
    //  stats_->mutations += off.mutation();

    if (*env.brood_recombination > 0)
    {
      fitness_t fit_off(this->eva_.fast(off));

      unsigned i(0);
      do
      {
        T tmp(pop[r1].crossover(pop[r2]));

        while (pop[r1].signature() == tmp.signature() ||
               pop[r2].signature() == tmp.signature())
          this->stats_->mutations += tmp.mutation();

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
    this->stats_->mutations += off.mutation();

    assert(off.debug());
    return {off};
  }
}

///
/// \param[in] parent a vector of ordered parents.
/// \return the offspring.
///
template<class T>
std::vector<T> de<T>::run(const std::vector<coord> &parent)
{
  assert(parent.size() == 3);

  const auto &pop(this->pop_);
  const auto &env(pop.env());

  assert(env.p_cross >= 0.0);

  return {pop[parent[0]].crossover(pop[parent[1]], pop[parent[2]])};
}
#endif  // Include guard
