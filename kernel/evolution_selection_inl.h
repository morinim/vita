/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(EVOLUTION_SELECTION_INL_H)
#define      EVOLUTION_SELECTION_INL_H

///
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
///
template<class T>
strategy<T>::strategy(const population<T> &pop, evaluator<T> &eva)
  : pop_(pop), eva_(eva)
{
}

///
/// \return the index of a random individual.
///
template<class T>
coord strategy<T>::pickup() const
{
  if (pop_.layers() == 1)
    return {0, vita::random::sup(pop_.individuals(0))};

  const auto layer(vita::random::sup(pop_.layers()));
  return {layer, vita::random::sup(pop_.individuals(layer))};
}

///
/// \param[in] target coordinates of a reference individual.
/// \return the coordinates of a random individual "near" \a target.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
///
template<class T>
coord strategy<T>::pickup(coord target) const
{
  return {target.layer, vita::random::ring(target.index, *pop_.env().mate_zone,
                                           pop_.individuals(target.layer))};
}

///
/// \param[in] l a layer.
/// \param[in] p the probability of extracting an individual in layer \a l
///              (1 - \a p is the probability of extracting an individual
///              in layer \a l-1).
/// \return the coordinates of a random individual in layer \a l.
///
template<class T>
coord strategy<T>::pickup(unsigned l, double p) const
{
  assert(0.0 <= p && p <= 1.0);

  assert(l < pop_.layers());

  if (l > 0 && !vita::random::boolean(p))
    --l;

  return {l, vita::random::sup(pop_.individuals(l))};
}

///
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
///
template<class T>
tournament<T>::tournament(const population<T> &pop, evaluator<T> &eva)
  : strategy<T>(pop, eva)
{
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
  // DO NOT USE std::sort it is way slower.
  for (unsigned i(0); i < rounds; ++i)
  {
    const auto new_coord(this->pickup(target));
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
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
///
template<class T>
alps<T>::alps(const population<T> &pop, evaluator<T> &eva)
  : strategy<T>(pop, eva)
{
}

///
///
///
template<class T>
std::vector<coord> alps<T>::run()
{
  const auto &pop(this->pop_);

  const auto layer(vita::random::sup(pop.layers()));

  auto c0(this->pickup(layer));
  auto c1(this->pickup(layer));

  typedef std::pair<bool, fitness_t> age_fit_t;
  age_fit_t age_fit0{!pop.aged(c0), this->eva_(pop[c0])};
  age_fit_t age_fit1{!pop.aged(c1), this->eva_(pop[c1])};

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
    const age_fit_t tmp_age_fit{!pop.aged(tmp), this->eva_(pop[tmp])};

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

    assert(age_fit0.first == !pop.aged(c0));
    assert(age_fit1.first == !pop.aged(c1));
    assert(age_fit0.second == this->eva_(pop[c0]));
    assert(age_fit1.second == this->eva_(pop[c1]));
    assert(age_fit0 >= age_fit1);
    assert(!pop.aged(c0) || pop.aged(c1));
    assert(layer <= c0.layer + 1);
    assert(layer <= c1.layer + 1);
    assert(c0.layer <= layer);
    assert(c1.layer <= layer);
  }

  return {c0, c1};
}

///
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
///
template<class T>
pareto<T>::pareto(const population<T> &pop, evaluator<T> &eva)
  : strategy<T>(pop, eva)
{
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

  std::set<unsigned> front, dominated;
  this->front(pool, &front, &dominated);

  assert(front.size());

  std::vector<coord> ret{
    {0, vita::random::element(front)},
    {0, vita::random::element(front)}};

  if (dominated.size())
    ret.push_back({0, vita::random::element(dominated)});

  return ret;
}

///
/// \param[in] pool indexes of individuals in the population.
/// \param[out] front the set of nondominated individuals of \a pool.
/// \param[out] dominated the set of dominated individuals of \a pool.
///
template<class T>
void pareto<T>::front(const std::vector<unsigned> &pool,
                      std::set<unsigned> *front,
                      std::set<unsigned> *dominated) const
{
  const auto &pop(this->pop_);

  for (const auto &ind : pool)
  {
    if (front->find(ind) != front->end() ||
        dominated->find(ind) != dominated->end())
      continue;

    const auto ind_fit(this->eva_(pop[{0, ind}]));

    bool ind_dominated(false);
    for (auto f(front->cbegin()); f != front->cend() && !ind_dominated;)
      // no increment in the for loop
    {
      const auto f_fit(this->eva_(pop[{0, *f}]));

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
/// \param[in] pop current population.
/// \param[in] eva current evaluator.
///
template<class T>
random<T>::random(const population<T> &pop, evaluator<T> &eva)
  : strategy<T>(pop, eva)
{
}

///
/// \return a vector of coordinates of randomly chosen individuals.
///
/// Parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population;
/// * tournament_size - to control number of selected individuals.
///
template<class T>
std::vector<coord> random<T>::run()
{
  const auto size(this->pop_.env().tournament_size);

  assert(size);
  std::vector<coord> ret(size);

  ret[0] = {0, this->pickup()};  // target

  for (unsigned i(1); i < size; ++i)
    ret[i] = {0, this->pickup(ret[0])};

  return ret;
}
#endif  // EVOLUTION_SELECTION_INL_H
