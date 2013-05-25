/**
 *
 *  \file evolution_selection_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_SELECTION_INL_H)
#define      EVOLUTION_SELECTION_INL_H

///
/// \param[in] evo pointer to the current evolution object.
///
template<class T>
selection_strategy<T>::selection_strategy(const evolution<T> *const evo)
  : evo_(evo)
{
}

///
/// \return the index of a random individual.
///
template<class T>
size_t selection_strategy<T>::pickup() const
{
  return random::between<size_t>(0, evo_->population().individuals());
}

///
/// \param[in] target index of a reference individual.
/// \return the index of a random individual "near" \a target.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
/// * tournament_size - to control number of selected individuals.
///
template<class T>
size_t selection_strategy<T>::pickup(size_t target) const
{
  const population<T> &pop(evo_->population());


  return random::ring(target, *pop.env().mate_zone, pop.individuals());
}

///
/// \param[in] evo pointer to the current evolution object.
///
template<class T>
tournament_selection<T>::tournament_selection(const evolution<T> *const evo)
  : selection_strategy<T>(evo)
{
}

/*
///
/// \param[in] target index of an \a individual in the \a population.
/// \return index of the best \a individual found.
///
/// Tournament selection works by selecting a number of individuals from the
/// population at random, a tournament, and then choosing only the best
/// of those individuals.
/// Recall that better individuals have highter fitnesses.
///
template<class T>
size_t tournament_selection<T>::tournament(size_t target) const
{
  const population<T> &pop(this->evo_->population());
  const unsigned rounds(pop.env().tournament_size);

  coord sel(pickup(target));
  for (unsigned i(1); i < rounds; ++i)
  {
    const coord j(pickup(target));

    const fitness_t fit_j(this->evo_->fitness(pop[j]));
    const fitness_t fit_sel(this->evo_->fitness(pop[sel]));
    if (fit_j > fit_sel)
    sel = j;
    }

  return sel;
}
*/

///
/// \return a vector of indexes of individuals ordered in descending
///         fitness.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
/// * tournament_size - to control selection pressure.
///
template<class T>
std::vector<size_t> tournament_selection<T>::run()
{
  const population<T> &pop(this->evo_->population());

  const auto rounds(pop.env().tournament_size);
  const size_t target(this->pickup());

  assert(rounds);
  std::vector<size_t> ret(rounds);

  // This is the inner loop of an insertion sort algorithm. It is simple,
  // fast (if rounds is small) and doesn't perform too much comparisons.
  // DO NOT USE std::sort it is way slower.
  for (unsigned i(0); i < rounds; ++i)
  {
    const size_t new_index(this->pickup(target));
    const auto new_fitness(this->evo_->fitness(pop[new_index]));

    size_t j(0);

    // Where is the insertion point?
    while (j < i && new_fitness < this->evo_->fitness(pop[ret[j]]))
      ++j;

    // Shift right elements after the insertion point.
    for (auto k(j); k < i; ++k)
      ret[k + 1] = ret[k];

    ret[j] = new_index;
  }

#if !defined(NDEBUG)
  for (size_t i(0); i + 1 < rounds; ++i)
    assert(this->evo_->fitness(pop[ret[i]]) >=
           this->evo_->fitness(pop[ret[i + 1]]));
#endif

  return ret;
}

///
/// \param[in] evo pointer to the current evolution object.
///
template<class T>
pareto_tourney<T>::pareto_tourney(const evolution<T> *const evo)
  : selection_strategy<T>(evo)
{
}

///
/// \return a vector of indexes of individuals partially ordered with the
///         pareto-dominance criterion.
///
/// Parameters from the environment:
/// * tournament_size - to control selection pressure (the number of randomly
///   selected individuals for dominance evaluation).
///
template<class T>
std::vector<size_t> pareto_tourney<T>::run()
{
  const population<T> &pop(selection_strategy<T>::evo_->population());
  const auto rounds(pop.env().tournament_size);

  std::vector<size_t> pool(rounds);
  for (unsigned i(0); i < rounds; ++i)
    pool.push_back(selection_strategy<T>::pickup());

  assert(pool.size());

  std::set<size_t> front, dominated;
  pareto(pool, &front, &dominated);

  assert(front.size());

  std::vector<size_t> ret{random::element(front), random::element(front)};

  if (dominated.size())
    ret.push_back(random::element(dominated));

  return ret;
}

///
/// \param[in] pool indexes of individuals in the population.
/// \param[out] front the set of nondominated individuals of \a pool.
/// \param[out] dominated the set of dominated individuals of \a pool.
///
template<class T>
void pareto_tourney<T>::pareto(const std::vector<size_t> &pool,
                               std::set<size_t> *front,
                               std::set<size_t> *dominated) const
{
  const population<T> &pop(this->evo_->population());

  for (const auto &ind : pool)
  {
    if (front->find(ind) != front->end() ||
        dominated->find(ind) != dominated->end())
      continue;

    const auto ind_fit(this->evo_->fitness(pop[ind]));

    bool ind_dominated(false);
    for (auto f(front->cbegin()); f != front->cend() && !ind_dominated;)
      // no increment in the for loop
    {
      const auto f_fit(this->evo_->fitness(pop[*f]));

      if (!ind_dominated && ind_fit.dominating(f_fit))
      {
        dominated->insert(*f);
        f = front->erase(f);
      }
      else
      {
        if (f_fit.dominating(ind_fit))
        {
          ind_dominated = true;
          dominated->insert(ind);
        }

        ++f;
      }
    }

    if (!ind_dominated)
      front->insert(ind);
  }
}

///
/// \param[in] evo pointer to the current evolution object.
///
template<class T>
random_selection<T>::random_selection(const evolution<T> *const evo)
  : selection_strategy<T>(evo)
{
}

///
/// \return a vector of indexes to individuals randomly chosen.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
/// * tournament_size - to control number of selected individuals.
///
template<class T>
std::vector<size_t> random_selection<T>::run()
{
  const size_t size(this->evo_->population().env().tournament_size);

  assert(size);
  std::vector<size_t> ret(size);

  ret[0] = this->pickup();  // target

  for (size_t i(1); i < size; ++i)
    ret[i] = this->pickup(ret[0]);

  return ret;
}
#endif  // EVOLUTION_SELECTION_INL_H
