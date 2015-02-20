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

#if !defined(VITA_EVOLUTION_SELECTION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_SELECTION_TCC)
#define      VITA_EVOLUTION_SELECTION_TCC

///
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
/// \param[in] sum up to date summary of the evolution.
///
template<class T>
strategy<T>::strategy(const population<T> &pop, evaluator<T> &eva,
                      const summary<T> &sum)
  : pop_(pop), eva_(eva), sum_(sum)
{
}

///
/// \return the index of a random individual.
///
template<class T>
coord strategy<T>::pickup() const
{
  const auto n_layers(pop_.layers());

  if (n_layers == 1)
    return {0, vita::random::sup(pop_.individuals(0))};

  // If we have multiple layers we cannot be sure that every layer has the
  // same number of individuals. So the simple (and faster) solution:
  //
  //   const auto l(vita::random::sup(n_layers));
  //
  // isn't appropriate.
  std::vector<unsigned> s(n_layers);
  for (auto l(decltype(n_layers){0}); l < n_layers; ++l)
    s[l] = pop_.individuals(l);

  std::discrete_distribution<unsigned> dd(s.begin(), s.end());
  const auto l(dd(vita::random::engine()));
  return {l, vita::random::sup(s[l])};
}

///
/// \param[in] l a layer.
/// \param[in] p the probability of extracting an individual in layer \a l
///              (1 - \a p is the probability of extracting an individual
///              in layer \a l-1).
/// \return the coordinates of a random individual in layer \a l or \a l-1.
///
template<class T>
coord strategy<T>::pickup(unsigned l, double p) const
{
  assert(0.0 <= p);
  assert(p <= 1.0);
  assert(l < pop_.layers());

  if (l > 0 && !vita::random::boolean(p))
    --l;

  return {l, vita::random::sup(pop_.individuals(l))};
}

///
/// \return a vector of coordinates of individuals ordered in descending
///         fitness.
///
/// Tournament selection works by selecting a number of individuals from the
/// population at random, a tournament, and then choosing only the best
/// of those individuals.
/// Recall that better individuals have highter fitnesses.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
/// * tournament_size - to control selection pressure.
///
template<class T>
std::vector<coord> tournament<T>::run()
{
  const auto &pop(this->pop_);

  const auto rounds(pop.env().tournament_size);
  const auto target(this->pickup());

  assert(rounds);
  std::vector<coord> ret(rounds);

  // This is the inner loop of an insertion sort algorithm. It is simple,
  // fast (if rounds is small) and doesn't perform too much comparisons.
  // DO NOT USE std::sort it's way slower.
  for (unsigned i(0); i < rounds; ++i)
  {
    const auto new_coord(pickup(pop, target));
    const auto new_fitness(this->eva_(pop[new_coord]));

    unsigned j(0);

    // Where is the insertion point?
    while (j < i && new_fitness < this->eva_(pop[ret[j]]))
      ++j;

    // Shift right elements after the insertion point.
    for (auto k(j); k < i; ++k)
      ret[k + 1] = ret[k];

    ret[j] = new_coord;
  }

#if !defined(NDEBUG)
  for (unsigned i(1); i < rounds; ++i)
    assert(this->eva_(pop[ret[i - 1]]) >= this->eva_(pop[ret[i]]));
#endif

  return ret;
}

///
/// \return a vector of coordinates of chosen individuals.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
/// * tournament_size - to control number of selected individuals.
///
template<class T>
std::vector<coord> alps<T>::run()
{
  const auto &pop(this->pop_);

  const auto layer(vita::random::sup(pop.layers()));

  auto c0(this->pickup(layer));
  auto c1(this->pickup(layer));

  // This type is mainly used because of the lexicographic comparison
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

  const auto same_layer_p(pop.env().alps.p_same_layer);
  auto rounds(pop.env().tournament_size);

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
    assert(layer <= c0.layer + 1);
    assert(layer <= c1.layer + 1);
    assert(c0.layer <= layer);
    assert(c1.layer <= layer);
  }

  return {c0, c1};
}

///
/// \return a vector of coordinates of individuals with fitness nearest to a
///         random fitness value.
///
/// A random fitness value is chosen in the interval \f$[f_min, f_max]\f$,
/// where \f$f_max\f$ and \f$f_min\f$ are the maximum and minimum fitness value
/// in the current population.
/// Then we round a tournament to find the individuals with fitness nearest to
/// this random value.
/// While the probability of selection each fitness level is equal,
/// the probability of then selecting a given individual within a fitness level
/// depends on the population of that level.
///
template<class T>
std::vector<coord> fuss<T>::run()
{
  const auto &pop(this->pop_);

  const auto rounds(pop.env().tournament_size);
  assert(rounds);

  const auto min(this->sum_.az.fit_dist().min);
  const auto max(this->sum_.az.fit_dist().max);
  auto level(max - min);

  for (unsigned i(0); i < decltype(level)::size; ++i)
  {
    const auto base(std::min(min[i], max[i]));
    const auto delta(std::fabs(level[i]));

    level[i] = base + vita::random::between<decltype(base)>(-0.5, delta + 0.5);

    assert(std::min(min[i], max[i]) - 0.5 <= level[i]);
    assert(level[i] <= std::max(min[i], max[i]) + 0.5);
  }

  std::vector<coord> ret(rounds);

  // This is the inner loop of an insertion sort algorithm. It is simple,
  // fast (if rounds is small) and doesn't perform too much comparisons.
  // DO NOT USE std::sort it is way slower.
  for (unsigned i(0); i < rounds; ++i)
  {
    const auto new_coord(this->pickup());
    const auto new_fit(this->eva_(pop[new_coord]));

    unsigned j(0);

    // Where is the insertion point?
    while (j < i &&
           level.distance(new_fit) > level.distance(this->eva_(pop[ret[j]])))
      ++j;

    // Shift right elements after the insertion point.
    for (auto k(j); k < i; ++k)
      ret[k + 1] = ret[k];

    ret[j] = new_coord;
  }

#if !defined(NDEBUG)
  const auto size(ret.size());
  for (auto i(decltype(size){1}); i < size; ++i)
    assert(level.distance(this->eva_(pop[ret[i - 1]])) <=
           level.distance(this->eva_(pop[ret[i]])));
#endif

  return ret;
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
std::vector<coord> pareto<T>::run()
{
  const auto &pop(this->pop_);
  const auto rounds(pop.env().tournament_size);

  std::vector<unsigned> pool(rounds);
  for (unsigned i(0); i < rounds; ++i)
    pool.push_back(this->pickup());

  assert(pool.size());

  std::set<unsigned> front_set, dominated_set;
  this->front(pool, &front_set, &dominated_set);

  assert(front_set.size());

  std::vector<coord> ret{
    {0, vita::random::element(front_set)},
    {0, vita::random::element(front_set)}};

  if (dominated_set.size())
    ret.push_back({0, vita::random::element(dominated_set)});

  return ret;
}

///
/// \param[in] pool indexes of individuals in the population.
/// \param[out] front the set of nondominated individuals of \a pool.
/// \param[out] dominated the set of dominated individuals of \a pool.
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
std::vector<coord> random<T>::run()
{
  const auto size(this->pop_.env().tournament_size);

  assert(size);
  std::vector<coord> ret(size);

  for (auto &v : ret)
    v = this->pickup();

  return ret;
}
#endif  // Include guard
