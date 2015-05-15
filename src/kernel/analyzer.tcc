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

#if !defined(VITA_ANALYZER_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_ANALYZER_TCC)
#define      VITA_ANALYZER_TCC

///
/// New empty core_analyzer.
///
template<class T>
core_analyzer<T>::core_analyzer()
{
  clear();
}

///
/// Resets gathered statics.
///
template<class T>
void core_analyzer<T>::clear()
{
  age_.clear();
  fit_.clear();
  length_.clear();

  functions_ = sym_counter();
  terminals_ = sym_counter();

  sym_counter_.clear();
  layer_stat_.clear();
}

///
/// \return a constant reference to the first statistical \a symbol we have
///         informations about.
///
template<class T>
typename core_analyzer<T>::const_iterator core_analyzer<T>::begin() const
{
  return sym_counter_.begin();
}

///
/// \return a constant reference (sentry) used for loops.
///
template<class T>
typename core_analyzer<T>::const_iterator core_analyzer<T>::end() const
{
  return sym_counter_.end();
}

///
/// \param[in] eff effective / noneffective functions.
/// \return number of functions in the population.
///
template<class T>
std::uintmax_t core_analyzer<T>::functions(bool eff) const
{
  return functions_.counter[eff];
}

///
/// \param[in] eff effective / noneffective terminals.
/// \return number of terminals in the population.
///
template<class T>
std::uintmax_t core_analyzer<T>::terminals(bool eff) const
{
  return terminals_.counter[eff];
}

///
/// \return statistics about the age distribution of the individuals.
///
template<class T>
const distribution<double> &core_analyzer<T>::age_dist() const
{
  assert(age_.debug(true));

  return age_;
}

///
/// \param[in] l a layer.
/// \return statistics about the age distribution of individuals in layer
///         \a l.
///
template<class T>
const distribution<double> &core_analyzer<T>::age_dist(unsigned l) const
{
  assert(layer_stat_.find(l) != layer_stat_.end());
  assert(layer_stat_.find(l)->second.age.debug(true));

  return layer_stat_.find(l)->second.age;
}

///
/// \return statistics about the fitness distribution of the individuals.
///
template<class T>
const distribution<fitness_t> &core_analyzer<T>::fit_dist() const
{
  assert(fit_.debug(true));

  return fit_;
}

///
/// \param[in] l a layer.
/// \return statistics about the fitness distribution of individuals in layer
///         \a l.
///
template<class T>
const distribution<fitness_t> &core_analyzer<T>::fit_dist(unsigned l) const
{
  assert(layer_stat_.find(l) != layer_stat_.end());
  assert(layer_stat_.find(l)->second.fitness.debug(true));

  return layer_stat_.find(l)->second.fitness;
}

///
/// \return statistic about the length distribution of the individuals.
///
template<class T>
const distribution<double> &core_analyzer<T>::length_dist() const
{
  assert(length_.debug(true));

  return length_;
}

///
/// \param[in] sym symbol we are gathering statistics about.
/// \param[in] active is this an active gene?
///
/// Used by core_analyzer<T>::count(const T &)
///
template<class T>
void core_analyzer<T>::count(const symbol *const sym, bool active)
{
  assert(sym);

  ++sym_counter_[sym].counter[active];

  if (sym->terminal())
    ++terminals_.counter[active];
  else
    ++functions_.counter[active];
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T>
bool core_analyzer<T>::debug() const
{
  for (const auto &i : sym_counter_)
    if (i.second.counter[true] > i.second.counter[false])
      return false;

  if (!age_.debug(true))
    return false;

  if (!fit_.debug(true))
    return false;

  return length_.debug(true);
}

///
/// \param[in] ind new individual.
/// \param[in] f fitness of the new individual.
/// \param[in] l a layer of the population.
///
/// Adds a new individual to the pool used to calculate statistics.
///
template<class T>
void core_analyzer<T>::add(const T &ind, const fitness_t &f, unsigned l)
{
  age_.add(ind.age());
  layer_stat_[l].age.add(ind.age());

  length_.add(count(ind));

  if (isfinite(f))
  {
    fit_.add(f);
    layer_stat_[l].fitness.add(f);
  }
}

///
/// \tparam T type of individual.
///
/// \param[in] ind individual to be analyzed.
/// \return effective length of individual we gathered statistics about.
///
template<class T>
unsigned analyzer<T>::count(const T &ind)
{
  for (index_t i(0); i < ind.size(); ++i)
    for (category_t c(0); c < ind.env().sset->categories(); ++c)
      core_analyzer<T>::count(ind[{i, c}].sym, false);

  unsigned length(0);
  for (const auto &l : ind)
  {
    core_analyzer<T>::count(ind[l].sym, true);
    ++length;
  }

  return length;
}

///
/// \tparam T type of individual.
///
/// \param[in] t team to be analyzed.
/// \return effective length of the team we gathered statistics about.
///
template<class T>
unsigned analyzer<team<T>>::count(const team<T> &t)
{
  unsigned length(0);

  for (const auto &ind : t)
  {
    for (index_t i(0); i < ind.size(); ++i)
      for (category_t c(0); c < ind.env().sset->categories(); ++c)
        core_analyzer<team<T>>::count(ind[{i, c}].sym, false);

    for (const auto &l : ind)
    {
      core_analyzer<team<T>>::count(ind[l].sym, true);
      ++length;
    }
  }

  return length;
}

#endif  // Include guard
