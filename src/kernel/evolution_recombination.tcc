/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
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
/// \param[in] pop   the current population
/// \param[in] eva   the current evaluator
/// \param[in] stats pointer to the current set of statistics
///
template<class T>
strategy<T>::strategy(const population<T> &pop, evaluator<T> &eva,
                      summary<T> *stats)
  : pop_(pop), eva_(eva), stats_(stats)
{
  Expects(stats);
}

///
/// This is a quite standard crossover + mutation operator.
///
/// \param[in] parent a vector of ordered parents
/// \return           the offspring
///
template<class T>
typename strategy<T>::offspring_t base<T>::run(
  const typename strategy<T>::parents_t &parent)
{
  const auto &pop(this->pop_);
  const auto &prob(pop.get_problem());

  const auto p_cross(prob.env.p_cross);
  const auto p_mutation(prob.env.p_mutation);
  const auto brood_recombination(prob.env.brood_recombination);

  Expects(0.0 <= p_cross && p_cross <= 1.0);
  Expects(0.0 <= p_mutation && p_mutation <= 1.0);
  Expects(brood_recombination);
  Expects(parent.size() >= 2);

  const auto r1(parent[0]), r2(parent[1]);

  if (random::boolean(p_cross))
  {
    auto cross_and_mutate(
      [&](const T &p1, const T &p2)
      {
        T ret(crossover(p1, p2));
        ++this->stats_->crossovers;

        // This could be an original contribution of Vita but it's hard to be
        // sure.
        // It remembers of the hereditary repulsion constraint (I guess you
        // could call it signature repulsion) and seems to:
        // * maintain diversity during the exploration phase;
        // * optimize the exploitation phase.
        while (p1.signature() == ret.signature() ||
               p2.signature() == ret.signature())
          this->stats_->mutations += ret.mutation(p_mutation, prob);

        return ret;
      });

    T off(cross_and_mutate(pop[r1], pop[r2]));

    if (brood_recombination > 1)
    {
      fitness_t fit_off(this->eva_.fast(off));

      for (unsigned i(1); i < brood_recombination; ++i)
      {
        T tmp(cross_and_mutate(pop[r1], pop[r2]));

        const auto fit_tmp(this->eva_.fast(tmp));
        if (fit_tmp > fit_off)
        {
          off     =     tmp;
          fit_off = fit_tmp;
        }
      }
    }

    return {off};
  }

  // !crossover
  T off(pop[random::boolean() ? r1 : r2]);
  this->stats_->mutations += off.mutation(p_mutation, prob);

  return {off};
}

///
/// This is strictly based on the DE crossover operator.
///
/// \param[in] parent a vector of ordered parents
/// \return           the offspring
///
template<class T>
typename strategy<T>::offspring_t de<T>::run(
  const typename strategy<T>::parents_t &parent)
{
  Expects(parent.size() >= 2);

  const auto &pop(this->pop_);
  const auto &env(pop.get_problem().env);

  assert(0.0 < env.p_cross);
  assert(env.p_cross <= 1.0);

  const auto a(pickup(pop, parent[0]));
  const auto b(pickup(pop, parent[0]));

  return {pop[parent[0]].crossover(env.p_cross, env.de.weight,
                                   pop[parent[1]], pop[a], pop[b])};
}
#endif  // include guard
