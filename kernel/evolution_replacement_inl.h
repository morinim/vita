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
/// \param[in] evo pointer to the current evolution object.
///
template<class T>
replacement_strategy<T>::replacement_strategy(evolution<T> *const evo)
  : evo_(evo)
{
}

///
/// \param[in] evo pointer to the current evolution object.
///
template<class T>
family_competition_rp<T>::family_competition_rp(evolution<T> *const evo)
  : replacement_strategy<T>(evo)
{
}

///
/// \param[in] parent indexes of the parents (in the population).
/// \param[in] offspring vector of the "children".
/// \param[in] s statistical \a summary.
///
/// Parameters from the environment:
/// * elitism is \c true => child replaces a member of the population only if
///   child is better.
///
template<class T>
void family_competition_rp<T>::run(const std::vector<size_t> &parent,
                                   const std::vector<T> &offspring,
                                   summary<T> *const s)
{
  population<T> &pop(replacement_strategy<T>::evo_->population());
  assert(!boost::indeterminate(pop.env().elitism));

  const fitness_t fit_off(replacement_strategy<T>::evo_->fitness(offspring[0]));

  const fitness_t f_parent[] =
  {
    replacement_strategy<T>::evo_->fitness(pop[parent[0]]),
    replacement_strategy<T>::evo_->fitness(pop[parent[1]])
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
/// \param[in] evo pointer to the evolution object that is using the
///                kill_tournament.
///
template<class T>
kill_tournament<T>::kill_tournament(evolution<T> *const evo)
  : replacement_strategy<T>(evo)
{
}

///
/// \param[in] parent indexes of the candidate parents.
///                   The list is sorted in descending fitness, so the
///                   last element is the index of the worst individual of
///                   the tournament.
/// \param[in] offspring vector of the "children".
/// \param[in] s statistical \a summary.
///
/// Parameters from the environment:
/// * elitism is \c true => child replaces a member of the population only if
///   child is better.
///
template<class T>
void kill_tournament<T>::run(const std::vector<size_t> &parent,
                             const std::vector<T> &offspring,
                             summary<T> *const s)
{
  population<T> &pop(replacement_strategy<T>::evo_->population());

  const auto fit_off(replacement_strategy<T>::evo_->fitness(offspring[0]));

  // In old versions of Vita, the individual to be replaced was chosen with
  // an ad-hoc kill tournament. Something like:
  //
  //   const size_t rep_idx(kill_tournament(parent[0]));
  //
  // Now we perform just one tournament for choosing the parents; the
  // individual to be replaced is selected among the worst individuals of
  // this tournament.
  // The new way is simpler and more general. Note that when tournament_size
  // is greater than 2 we perform a traditional selection / replacement
  // scheme; if it is smaller we perform a family competition replacement
  // (aka deterministic / probabilistic crowding).
  const size_t rep_idx(parent.back());
  const auto f_rep_idx(replacement_strategy<T>::evo_->fitness(pop[rep_idx]));
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
/// \param[in] evo pointer to the evolution object that is using the
///                pareto_tournament.
///
template<class T>
pareto_tournament<T>::pareto_tournament(evolution<T> *const evo)
  : replacement_strategy<T>(evo)
{
}

///
/// \param[in] parent indexes of the candidate parents.
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
void pareto_tournament<T>::run(const std::vector<size_t> &parent,
                               const std::vector<T> &offspring,
                               summary<T> *const s)
{
  population<T> &pop(replacement_strategy<T>::evo_->population());

  const auto fit_off(replacement_strategy<T>::evo_->fitness(offspring[0]));
/*
  for (auto i(parent.rbegin()); i != parent.rend(); ++i)
  {
    const auto fit_i(replacement_strategy<T>::evo_->fitness(pop[*i]));

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
    const auto fit_i(replacement_strategy<T>::evo_->fitness(pop[i]));

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
