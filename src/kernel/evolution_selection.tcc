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

#if !defined(VITA_EVOLUTION_SELECTION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_SELECTION_TCC)
#define      VITA_EVOLUTION_SELECTION_TCC

///
/// \param[in] pop current population
/// \param[in] eva current evaluator
/// \param[in] sum up to date summary of the evolution
///
template<class T>
strategy<T>::strategy(const population<T> &pop, evaluator<T> &eva,
                      const summary<T> &sum)
  : pop_(pop), eva_(eva), sum_(sum)
{
}

///
/// \return a collection of coordinates of individuals ordered in descending
///         fitness
///
/// Tournament selection works by selecting a number of individuals from the
/// population at random (a tournament) and then choosing only the best of
/// those individuals.
/// Recall that better individuals have higher fitness.
///
/// Parameters from the environment:
/// * `mate_zone` - to restrict the selection of individuals to a segment of
///   the population;
/// * `tournament_size` - to control selection pressure.
///
/// \remark
/// Different compilers may optimize the code producing slightly different
/// sortings (due to floating point approximations). This is a known *issue*.
/// Anyway we keep using the `<` operator because:
/// - it's faster than the `std::fabs(delta)` approach;
/// - the additional *noise* is marginal (for the GAs/GP standard);
/// - for debugging purposes *compiler-stability* is enough (and we have faith
///   in the test suite).
///
template<class T>
typename strategy<T>::parents_t tournament<T>::run()
{
  const auto &pop(this->pop_);

  const auto rounds(pop.get_problem().env.tournament_size);
  assert(rounds);

  const auto target(pickup(pop));
  typename strategy<T>::parents_t ret(rounds);

  // This is the inner loop of an insertion sort algorithm. It's simple, fast
  // (if `rounds` is small) and doesn't perform too much comparisons.
  // DO NOT USE `std::sort` it's way slower.
  for (unsigned i(0); i < rounds; ++i)
  {
    const auto new_coord(pickup(pop, target));
    const auto new_fitness(this->eva_(pop[new_coord]));

    auto j(i);

    for (; j && new_fitness > this->eva_(pop[ret[j - 1]]); --j)
      ret[j] = ret[j - 1];

    ret[j] = new_coord;
  }

#if !defined(NDEBUG)
  assert(ret.size() == rounds);

  for (unsigned i(1); i < rounds; ++i)
    assert(this->eva_(pop[ret[i - 1]]) >= this->eva_(pop[ret[i]]));
#endif

  return ret;
}

///
/// \param[in] l a layer
/// \param[in] p the probability of extracting an individual in layer `l`
///              (`1 - p` is the probability of extracting an individual
///              in layer `l-1`)
/// \return      the coordinates of a random individual in layer `l` or `l-1`
///
template<class T>
typename population<T>::coord alps<T>::pickup(unsigned l, double p) const
{
  Expects(l < this->pop_.layers());
  Expects(0.0 <= p && p <= 1.0);

  if (l > 0 && !vita::random::boolean(p))
    --l;

  return {l, vita::random::sup(this->pop_.individuals(l))};
}

///
/// \return a vector of coordinates of chosen individuals
///
/// Parameters from the environment:
/// - `mate_zone` to restrict the selection of individuals to a segment of
///   the population;
/// - `tournament_size` to control number of selected individuals.
///
template<class T>
typename strategy<T>::parents_t alps<T>::run()
{
  const auto &pop(this->pop_);

  const auto layer(vita::random::sup(pop.layers()));

  auto c0(this->pickup(layer));
  auto c1(this->pickup(layer));

  // This type is used to take advantage of the lexicographic comparison
  // capabilities of std::pair.
  using age_fit_t = std::pair<bool, fitness_t>;
  age_fit_t age_fit0{!vita::alps::aged(pop, c0), this->eva_(pop[c0])};
  age_fit_t age_fit1{!vita::alps::aged(pop, c1), this->eva_(pop[c1])};

  if (age_fit0 < age_fit1)
  {
    std::swap(c0, c1);
    std::swap(age_fit0, age_fit1);
  }

  assert(age_fit0 >= age_fit1);

  const auto &env(pop.get_problem().env);
  const auto same_layer_p(env.alps.p_same_layer);
  auto rounds(env.tournament_size);

  while (rounds--)
  {
    const auto tmp(this->pickup(layer, same_layer_p));
    const age_fit_t tmp_age_fit{!vita::alps::aged(pop, tmp),
                                this->eva_(pop[tmp])};

    if (age_fit0 < tmp_age_fit)
    {
      c1 = c0;
      age_fit1 = age_fit0;

      c0 = tmp;
      age_fit0 = tmp_age_fit;
    }
    else if (age_fit1 < tmp_age_fit)
    {
      c1 = tmp;
      age_fit1 = tmp_age_fit;
    }

    assert(age_fit0.first == !vita::alps::aged(pop, c0));
    assert(age_fit1.first == !vita::alps::aged(pop, c1));
    assert(age_fit0.second == this->eva_(pop[c0]));
    assert(age_fit1.second == this->eva_(pop[c1]));
    assert(age_fit0 >= age_fit1);
    assert(!vita::alps::aged(pop, c0) || vita::alps::aged(pop, c1));
    assert(c0.layer <= layer);
    assert(layer <= c0.layer + 1);
    assert(c1.layer <= layer);
    assert(layer <= c1.layer + 1);
  }

  return {c0, c1};
}

///
/// \return a vector of coordinates of individuals partially ordered with the
///         pareto-dominance criterion.
///
/// Parameters from the environment:
/// * tournament_size - to control selection pressure (the number of randomly
///   selected individuals for dominance evaluation).
///
template<class T>
typename strategy<T>::parents_t pareto<T>::run()
{
  const auto &pop(this->pop_);
  const auto rounds(pop.env().tournament_size);

  std::vector<unsigned> pool(rounds);
  for (unsigned i(0); i < rounds; ++i)
    pool.push_back(pickup(pop));

  assert(pool.size());

  std::set<unsigned> front_set, dominated_set;
  this->front(pool, &front_set, &dominated_set);

  assert(front_set.size());

  typename strategy<T>::parents_t ret{
    {0, vita::random::element(front_set)},
    {0, vita::random::element(front_set)}};

  if (dominated_set.size())
    ret.push_back({0, vita::random::element(dominated_set)});

  return ret;
}

///
/// \param[in] pool indexes of individuals in the population.
/// \param[out] front the set of nondominated individuals of `pool`.
/// \param[out] dominated the set of dominated individuals of `pool.
///
template<class T>
void pareto<T>::front(const std::vector<unsigned> &pool,
                      std::set<unsigned> *fs, std::set<unsigned> *ds) const
{
  const auto &pop(this->pop_);

  for (const auto &ind : pool)
  {
    if (fs->find(ind) != fs->end() || ds->find(ind) != ds->end())
      continue;

    const auto ind_fit(this->eva_(pop[{0, ind}]));

    bool ind_dominated(false);
    for (auto f(fs->cbegin()); f != fs->cend() && !ind_dominated;)
      // no increment in the for loop
    {
      const auto f_fit(this->eva_(pop[{0, *f}]));

      if (!ind_dominated && ind_fit.dominating(f_fit))
      {
        ds->insert(*f);
        f = fs->erase(f);
      }
      else
      {
        if (f_fit.dominating(ind_fit))
        {
          ind_dominated = true;
          ds->insert(ind);
        }

        ++f;
      }
    }

    if (!ind_dominated)
      fs->insert(ind);
  }
}

///
/// \return a vector of coordinates of randomly chosen individuals.
///
/// Parameters from the environment:
/// * tournament_size - to control number of selected individuals.
///
template<class T>
typename strategy<T>::parents_t random<T>::run()
{
  const auto pop(this->pop_);
  const auto size(pop.get_problem().env.tournament_size);

  assert(size);
  typename strategy<T>::parents_t ret(size);

  for (auto &v : ret)
    v = pickup(pop);

  return ret;
}
#endif  // include guard
