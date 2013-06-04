/**
 *
 *  \file evolution_replacement_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_REPLACEMENT_INL_H)
#define      EVOLUTION_REPLACEMENT_INL_H

///
/// \param[in] e pointer to the current evolution object.
///
template<class T>
strategy<T>::strategy(evolution<T> *const e) : evo_(e)
{
}

///
/// \param[in] e pointer to the current evolution object.
///
template<class T>
family_competition<T>::family_competition(evolution<T> *const e)
  : strategy<T>(e)
{
}

///
/// \param[in] parent coordinates of the parents (in the population).
/// \param[in] offspring vector of the "children".
/// \param[in] s statistical \a summary.
///
/// Parameters from the environment:
/// * elitism is \c true => child replaces a member of the population only if
///   child is better.
///
template<class T>
void family_competition<T>::run(const std::vector<coord> &parent,
                                const std::vector<T> &offspring,
                                summary<T> *const s)
{
  population<T> &pop(this->evo_->population());
  assert(!boost::indeterminate(pop.env().elitism));

  const fitness_t fit_off(this->evo_->fitness(offspring[0]));

  const fitness_t f_parent[] =
  {
    this->evo_->fitness(pop[parent[0]]), this->evo_->fitness(pop[parent[1]])
  };
  const bool id_worst(f_parent[0] < f_parent[1] ? 0 : 1);

  if (pop.env().elitism)
  {
    if (fit_off > f_parent[id_worst])
      pop[parent[id_worst]] = offspring[0];
  }
  else  // !elitism
  {
    // THIS CODE IS APPROPRIATE ONLY WHEN FITNESS IS A SCALAR. It will work
    // when fitness is a vector but the replacement probability should be
    // calculated in a better way.

    //double replace(1.0 / (1.0 + exp(f_parent[id_worst][0] - fit_off[0])));
    double replace(1.0 - (fit_off[0] /
                          (fit_off[0] + f_parent[id_worst][0])));
    if (random::boolean(replace))
      pop[parent[id_worst]] = offspring[0];
    else
    {
      //replace = 1.0 / (1.0 + exp(f_parent[!id_worst][0] - fit_off[0]));
      replace = 1.0 - (fit_off[0] / (fit_off[0] + f_parent[!id_worst][0]));

      if (random::boolean(replace))
        pop[parent[!id_worst]] = offspring[0];
    }

    //pop[parent[id_worst]] = offspring[0];
  }

  if (fit_off > s->best->fitness)
  {
    typename summary<T>::best_ b{offspring[0], fit_off};
    s->last_imp = s->gen;
    s->best     =      b;
  }
}

///
/// \param[in] e pointer to the evolution object that is using the
///              (kill) tournament.
///
template<class T>
tournament<T>::tournament(evolution<T> *const e) : strategy<T>(e)
{
}

///
/// \param[in] parent coordinates of the candidate parents.
///                   The list is sorted in descending fitness, so the
///                   last element is the coordinates of the worst individual
///                   of the tournament.
/// \param[in] offspring vector of the "children".
/// \param[in] s statistical \a summary.
///
/// Parameters from the environment:
/// * elitism is \c true => child replaces a member of the population only if
///   child is better.
///
template<class T>
void tournament<T>::run(const std::vector<coord> &parent,
                        const std::vector<T> &offspring,
                        summary<T> *const s)
{
  population<T> &pop(this->evo_->population());

  const auto fit_off(this->evo_->fitness(offspring[0]));

  // In old versions of Vita, the individual to be replaced was chosen with
  // an ad-hoc kill tournament. Something like:
  //
  //   const coord rep_idx(kill_tournament(parent[0]));
  //
  // Now we perform just one tournament for choosing the parents; the
  // individual to be replaced is selected among the worst individuals of
  // this tournament.
  // The new way is simpler and more general. Note that when tournament_size
  // is greater than 2 we perform a traditional selection / replacement
  // scheme; if it is smaller we perform a family competition replacement
  // (aka deterministic / probabilistic crowding).
  const coord rep_idx(parent.back());
  const auto f_rep_idx(this->evo_->fitness(pop[rep_idx]));
  const bool replace(f_rep_idx < fit_off);

  assert(!boost::indeterminate(pop.env().elitism));
  if (!pop.env().elitism || replace)
    pop[rep_idx] = offspring[0];

  if (fit_off > s->best->fitness)
  {
    s->last_imp =                  s->gen;
    s->best     = {offspring[0], fit_off};
  }
}

///
/// \param[in] e pointer to the evolution object that is using the
///              alps replacement.
///
template<class T>
alps<T>::alps(evolution<T> *const e) : strategy<T>(e)
{
}

///
/// \param[in] layer a layer
/// \param[in] incoming an individual
///
/// We would like to add \a incoming in layer \a layer. The insertion will take
/// place if:
/// * \a layer is not full or...
/// *
///
template<class T>
void alps<T>::try_add_to_layer(unsigned layer, const T &incoming)
{
  auto &p(this->evo_->population());
  assert(layer < p.layers());

  if (p.individuals(layer) < p.env().individuals)
    p.add_to_layer(layer, incoming);  // layer not full... inserting incoming
  else
  {
    coord c_worst{layer, random::sup(p.individuals(layer))};
    fitness_t f_worst(this->evo_->fitness(p[c_worst]));

    auto rounds(p.env().tournament_size);
    while (rounds--)
    {
      const coord c_candidate{layer, random::sup(p.individuals(layer))};
      const fitness_t f_candidate(this->evo_->fitness(p[c_candidate]));

      if (p[c_worst].age < p[c_candidate].age ||
          (p[c_worst].age == p[c_candidate].age && f_worst > f_candidate))
      {
        c_worst = c_candidate;
        f_worst = f_candidate;
      }
    }

    const auto max_age(p.max_age(layer));
    if ((p[c_worst].age > max_age && incoming.age <= max_age) ||
        this->evo_->fitness(incoming) >= f_worst)
    {
      if (layer + 1 < p.layers())
        try_add_to_layer(layer + 1, p[c_worst]);
      p[c_worst] = incoming;
    }
  }
}

///
/// \param[in] parent coordinates of the candidate parents.
///                   The list is sorted in descending fitness, so the
///                   last element is the coordinates of the worst individual
///                   of the tournament.
/// \param[in] offspring vector of the "children".
/// \param[in] s statistical \a summary.
///
/// Parameters from the environment:
/// * elitism is \c true => child replaces a member of the population only if
///   child is better.
///
template<class T>
void alps<T>::run(const std::vector<coord> &parent,
                  const std::vector<T> &offspring,
                  summary<T> *const s)
{
  const auto layer(std::max(parent[0].layer, parent[1].layer));

  try_add_to_layer(layer, offspring[0]);

  const fitness_t f_off(this->evo_->fitness(offspring[0]));
  if (f_off > s->best->fitness)
  {
    s->last_imp =                s->gen;
    s->best     = {offspring[0], f_off};
  }
}

///
/// \param[in] e pointer to the evolution object that is using the
///              pareto tournament.
///
template<class T>
pareto<T>::pareto(evolution<T> *const e) : strategy<T>(e)
{
}

///
/// \param[in] parent coordinates of the candidate parents.
///                   The list is sorted in descending pareto-layer
///                   dominance (from pareto non dominated front to
///                   dominated points.
/// \param[in] offspring vector of the "children".
/// \param[in] s statistical \a summary.
///
/// To determine whether a new individual x is to be accepted into the main
/// population, we compare it with the \a parent buffer, simply ensuring
/// that the new individual is not dominated.
///
/// If this is the case, then it is immediately accepted and is inserted
/// according to the replacement rules. The only parameter that needs to be
/// determined in advance is the tournament size, a parameter that would
/// exist in a single objective optimisation anyway.
///
/// Parameters from the environment:
/// * elitism is \c true => child replaces a member of the population only if
///   child is better.
///
/// \see
/// "A Robust Evolutionary Technique for Coupled and Multidisciplinary Design
/// Optimization problems in Aeronautics" - L.F.Gonzalez, E.J. Whithney,
/// K. Srinivas, S. Armfield, J. Periaux.
///
template<class T>
void pareto<T>::run(const std::vector<coord> &parent,
                    const std::vector<T> &offspring,
                    summary<T> *const s)
{
  population<T> &pop(this->evo_->population());

  const auto fit_off(this->evo_->fitness(offspring[0]));
/*
  for (auto i(parent.rbegin()); i != parent.rend(); ++i)
  {
    const auto fit_i(this->evo_->fitness(pop[*i]));

    if (fit_off.dominating(fit_i))
    {
      pop[*i] = offspring[0];

      if (fit_off[0] > s->best->fitness[0])
      {
        s->last_imp =                  s->gen;
        s->best     = {offspring[0], fit_off};
      }

      break;
    }
  }
*/

  bool dominated(false);
  for (const auto &i : parent)
  {
    const auto fit_i(this->evo_->fitness(pop[i]));

    if (fit_i.dominating(fit_off))
    {
      dominated = true;
      break;
    }
  }

  assert(!boost::indeterminate(pop.env().elitism));

  if (!pop.env().elitism || !dominated)
    pop[parent.back()] = offspring[0];

  if (fit_off[0] > s->best->fitness[0])
  {
    s->last_imp =                  s->gen;
    s->best     = {offspring[0], fit_off};
  }
}
#endif  // EVOLUTION_REPLACEMENT_INL_H
