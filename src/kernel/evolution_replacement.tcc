/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_REPLACEMENT_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_REPLACEMENT_TCC)
#define      VITA_EVOLUTION_REPLACEMENT_TCC

///
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
///
template<class T>
strategy<T>::strategy(population<T> &pop, evaluator<T> &eva) : pop_(pop),
                                                               eva_(eva)
{
}

///
/// \param[in] parent    coordinates of the parents (in the population).
/// \param[in] offspring vector of the "children".
/// \param[in,out] s     statistical summary.
///
/// Parameters from the environment:
/// * elitism is `true` => child replaces a member of the population only if
///   child is better.
///
template<class T>
void family_competition<T>::run(
  const typename strategy<T>::parents_t &parent,
  const typename strategy<T>::offspring_t &offspring, summary<T> *s)
{
  auto &pop(this->pop_);
  const auto elitism(pop.get_problem().env.elitism);
  Expects(elitism != trilean::unknown);

  const fitness_t fit_off(this->eva_(offspring[0]));

  const fitness_t fit_parent[] =
  {
    this->eva_(pop[parent[0]]), this->eva_(pop[parent[1]])
  };
  const unsigned id_worst(fit_parent[0] < fit_parent[1] ? 0 : 1);

  // The algorithm assumes that fitness values have same sign.
  assert((fit_off[0] <= 0.0) == (fit_parent[0][0] <= 0.0));
  assert((fit_off[0] <= 0.0) == (fit_parent[1][0] <= 0.0));

  if (elitism == trilean::yes)
  {
    if (fit_off > fit_parent[id_worst])
      pop[parent[id_worst]] = offspring[0];
  }
  else  // !elitism
  {
    // THIS CODE IS APPROPRIATE ONLY WHEN FITNESS IS A SCALAR. It compiles when
    // fitness is a vector but the replacement probability should be calculated
    // in a better way.

    //double replace(1.0 / (1.0 + exp(fit_parent[id_worst][0] - fit_off[0])));
    double replace(1.0 - (fit_off[0]
                          / (fit_off[0] + fit_parent[id_worst][0])));
    if (random::boolean(replace))
      pop[parent[id_worst]] = offspring[0];
    else
    {
      //replace = 1.0 / (1.0 + exp(f_parent[!id_worst][0] - fit_off[0]));
      replace = 1.0 - (fit_off[0] / (fit_off[0] + fit_parent[!id_worst][0]));

      if (random::boolean(replace))
        pop[parent[!id_worst]] = offspring[0];
    }
  }

  if (fit_off > s->best.score.fitness)
  {
    s->last_imp           = s->gen;
    s->best.solution      = offspring[0];
    s->best.score.fitness = fit_off;
  }
}

///
/// \param[in] parent coordinates of the candidate parents. Many selection
///                   algorithms sort the vector in descending fitness (with
///                   some exceptions, e.g. selection::random).
///                   Anyway here we assume that the last element contains the
///                   coordinates of the worst individual of the selection
///                   phase.
/// \param[in] offspring vector of the "children".
/// \param[in,out] s statistical summary.
///
/// Parameters from the environment:
/// * elitism is `true` => child replaces a member of the population only if
///   child is better.
///
template<class T>
void tournament<T>::run(
  const typename strategy<T>::parents_t &parent,
  const typename strategy<T>::offspring_t &offspring, summary<T> *s)
{
  auto &pop(this->pop_);
  const auto elitism(pop.get_problem().env.elitism);
  Expects(elitism != trilean::unknown);

  const auto fit_off(this->eva_(offspring[0]));

  // In old versions of Vita, the individual to be replaced was chosen with
  // an ad-hoc kill tournament.
  // Now we perform just one tournament for choosing the parents; the
  // individual to be replaced is selected among the worst individuals of
  // this tournament.
  // The new way is simpler and more general. Note that when tournament_size
  // is greater than 2 we perform a traditional selection / replacement
  // scheme; if it's smaller we perform a family competition replacement
  // (aka deterministic / probabilistic crowding).
  const auto rep_idx(parent.back());
  const auto f_rep_idx(this->eva_(pop[rep_idx]));
  const bool replace(f_rep_idx < fit_off);

  if (elitism == trilean::no || replace)
    pop[rep_idx] = offspring[0];

  if (fit_off > s->best.score.fitness)
  {
    s->last_imp           = s->gen;
    s->best.solution      = offspring[0];
    s->best.score.fitness = fit_off;
  }
}

///
/// \param[in] l a layer
/// \return the maximum allowed age for an individual in layer `l`
///
/// This is just a convenience method to save some keystroke.
///
template<class T>
unsigned alps<T>::allowed_age(unsigned l) const
{
  return vita::alps::allowed_age(this->pop_, l);
}

///
/// \param[in] l a layer.
///
/// Try to move individuals in layer `l` in the upper layer (calling
/// try_add_to_layer for each individual).
///
template<class T>
void alps<T>::try_move_up_layer(unsigned l)
{
  auto &pop(this->pop_);

  if (l + 1 < pop.layers())
  {
    const auto n(pop.individuals(l));

    for (auto i(decltype(n){0}); i < n; ++i)
      try_add_to_layer(l + 1, pop[{l, i}]);
  }
}

///
/// \param[in] layer a layer
/// \param[in] incoming an individual
///
/// We would like to add `incoming` in layer `layer`. The insertion will
/// take place if:
/// * `layer` is not full or...
/// * after a "kill tournament" selection, the worst individual found is
///   too old for `layer` while the incoming one is within the limits or...
/// * the worst individual has a lower fitness than the incoming one and
///   both are simultaneously within/outside the time frame of `layer`.
///
template<class T>
bool alps<T>::try_add_to_layer(unsigned layer, const T &incoming)
{
  using coord = typename population<T>::coord;

  auto &p(this->pop_);
  assert(layer < p.layers());

  if (p.individuals(layer) < p.allowed(layer))
  {
    p.add_to_layer(layer, incoming);  // layer not full... inserting incoming
    return true;
  }

  // Layer is full, can we replace an existing individual?
  const auto m_age(allowed_age(layer));

  // Well, let's see if the worst individual we can find with a tournament...
  coord c_worst{layer, random::sup(p.individuals(layer))};
  auto f_worst(this->eva_(p[c_worst]));

  auto rounds(p.get_problem().env.tournament_size);
  while (rounds--)
  {
    const coord c_x{layer, random::sup(p.individuals(layer))};
    const auto f_x(this->eva_(p[c_x]));

    if ((p[c_x].age() > p[c_worst].age() && p[c_x].age() > m_age) ||
        (p[c_worst].age() <= m_age && p[c_x].age() <= m_age &&
         f_x < f_worst))
    {
      c_worst = c_x;
      f_worst = f_x;
    }
  }

  // ... is worse than the incoming individual.
  if ((incoming.age() <= m_age && p[c_worst].age() > m_age) ||
      ((incoming.age() <= m_age || p[c_worst].age() > m_age) &&
       this->eva_(incoming) >= f_worst))
  {
    if (layer + 1 < p.layers())
      try_add_to_layer(layer + 1, p[c_worst]);
    p[c_worst] = incoming;

    return true;
  }

  return false;
}

///
/// \param[in] parent coordinates of the candidate parents.
///                   The list is sorted in descending fitness, so the
///                   last element is the coordinates of the worst individual
///                   of the tournament.
/// \param[in] offspring vector of the "children".
/// \param[in,out] s statistical summary.
///
/// Parameters from the environment:
/// * elitism is `true` => a new best individual is always inserted into the
///   population.
///
template<class T>
void alps<T>::run(
  const typename strategy<T>::parents_t &parent,
  const typename strategy<T>::offspring_t &offspring, summary<T> *s)
{
  const auto layer(std::max(parent[0].layer, parent[1].layer));
  const auto f_off(this->eva_(offspring[0]));
  const auto &pop(this->pop_);
  const auto elitism(pop.get_problem().env.elitism);

  Expects(elitism != trilean::unknown);

  bool ins;
#if defined(MUTUAL_IMPROVEMENT)
  // To protect the algorithm from the potential deleterious effect of intense
  // exploratory dynamics, we can use a constraint which mandate that an
  // individual must be better than both its parents before insertion into
  // the population.
  // See "Exploiting The Path of Least Resistance In Evolution" (Gearoid Murphy
  // and Conor Ryan).
  if (f_off > this->eva_(pop[parent[0]]) && f_off > this->eva_(pop[parent[1]]))
#endif
  {
    ins = try_add_to_layer(layer, offspring[0]);
  }

  if (f_off > s->best.score.fitness)
  {
    // Sometimes a new best individual is discovered in a lower layer but he is
    // too old for its layer and the random tournament may choose only "not
    // aged" individuals (i.e. individuals within the age limit of their layer).
    // When this happen the new best individual would be lost without the
    // command below.
    // There isn't an age limit for the last layer so try_add_to_layer will
    // always succeed.
    if (!ins && elitism == trilean::yes)
      try_add_to_layer(pop.layers() - 1, offspring[0]);

    s->last_imp           = s->gen;
    s->best.solution      = offspring[0];
    s->best.score.fitness = f_off;
  }
}

///
/// \param[in] parent coordinates of the candidate parents.
///                   The list is sorted in descending pareto-layer
///                   dominance (from pareto non dominated front to
///                   dominated points.
/// \param[in] offspring vector of the "children".
/// \param[in,out] s statistical summary.
///
/// To determine whether a new individual x is to be accepted into the main
/// population, we compare it with the `parent` buffer, simply ensuring
/// that the new individual is not dominated.
///
/// If this is the case, then it is immediately accepted and is inserted
/// according to the replacement rules. The only parameter that needs to be
/// determined in advance is the tournament size, a parameter that would
/// exist in a single objective optimisation anyway.
///
/// Parameters from the environment:
/// * elitism is `true` => child replaces a member of the population only if
///   child is better.
///
/// \see
/// "A Robust Evolutionary Technique for Coupled and Multidisciplinary Design
/// Optimization problems in Aeronautics" - L.F.Gonzalez, E.J. Whithney,
/// K. Srinivas, S. Armfield, J. Periaux.
///
template<class T>
void pareto<T>::run(
  const typename strategy<T>::parents_t &parent,
  const typename strategy<T>::offspring_t &offspring, summary<T> *s)
{
  auto &pop(this->pop_);
  const auto elitism(pop.get_problem().env.elitism);

  Expects(elitism != trilean::unknown);

  const auto fit_off(this->eva_(offspring[0]));
/*
  for (auto i(parent.rbegin()); i != parent.rend(); ++i)
  {
    const auto fit_i(this->eva_(pop[*i]));

    if (fit_off.dominating(fit_i))
    {
      pop[i] = offspring[0];

      if (fit_off > s->best.score.fitness)
      {
        s->last_imp           = s->gen;
        s->best.solution      = offspring[0];
        s->best.score.fitness = fit_off;
      }

      break;
    }
  }
*/

  bool dominated(false);
  for (const auto &i : parent)
  {
    const auto fit_i(this->eva_(pop[i]));

    if (fit_i.dominating(fit_off))
    {
      dominated = true;
      break;
    }
  }

  if (elitism == trilean::no || !dominated)
    pop[parent.back()] = offspring[0];

  if (fit_off > s->best.score.fitness)
  {
    s->last_imp           = s->gen;
    s->best.solution      = offspring[0];
    s->best.score.fitness = fit_off;
  }
}
#endif  // Include guard
