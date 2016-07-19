/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2016 EOS di Manlio Morini.
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
  group_stat_.clear();
}

///
/// \return a constant reference to the first statistical symbol we have
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
  Expects(age_.debug());

  return age_;
}

///
/// \param[in] g a group.
/// \return statistics about the age distribution of individuals in group `g`.
///
template<class T>
const distribution<double> &core_analyzer<T>::age_dist(unsigned g) const
{
  const auto gi(group_stat_.find(g));
  assert(gi != group_stat_.end());

  Ensures(gi->second.age.debug());
  return gi->second.age;
}

///
/// \return statistics about the fitness distribution of the individuals.
///
template<class T>
const distribution<fitness_t> &core_analyzer<T>::fit_dist() const
{
  Ensures(fit_.debug());
  return fit_;
}

///
/// \param[in] g a group.
/// \return statistics about the fitness distribution of individuals in group
///         `g`.
///
template<class T>
const distribution<fitness_t> &core_analyzer<T>::fit_dist(unsigned g) const
{
  const auto gi(group_stat_.find(g));
  assert(gi != group_stat_.end());

  Ensures(gi->second.fitness.debug());
  return gi->second.fitness;
}

///
/// \return statistic about the length distribution of the individuals.
///
template<class T>
const distribution<double> &core_analyzer<T>::length_dist() const
{
  Ensures(length_.debug());
  return length_;
}

///
/// \param[in] sym symbol we are gathering statistics about.
/// \param[in] active is this an active gene?
///
/// Used by `count(const T &)`.
///
template<class T>
void core_analyzer<T>::count(const symbol *sym, bool active)
{
  Expects(sym);

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

  if (!age_.debug())
    return false;

  if (!fit_.debug())
    return false;

  return length_.debug();
}

///
/// \param[in] ind new individual.
/// \param[in] f fitness of the new individual.
/// \param[in] g a group of the population.
///
/// Adds a new individual to the pool used to calculate statistics.
///
/// The optional `g` parameter can be used to group information (e.g. for the
/// ALPS algorithm it's used for layer specific statistics).
///
template<class T>
void core_analyzer<T>::add(const T &ind, const fitness_t &f, unsigned g)
{
  age_.add(ind.age());
  group_stat_[g].age.add(ind.age());

  length_.add(count(ind));

  if (isfinite(f))
  {
    fit_.add(f);
    group_stat_[g].fitness.add(f);
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
    for (category_t c(0); c < ind.categories(); ++c)
      core_analyzer<T>::count(ind[{i, c}].sym, false);

  unsigned length(0);
  for (const auto &g : ind)
  {
    core_analyzer<T>::count(g.sym, true);
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
      for (category_t c(0); c < ind.categories(); ++c)
        core_analyzer<team<T>>::count(ind[{i, c}].sym, false);

    for (const auto &g : ind)
    {
      core_analyzer<team<T>>::count(g.sym, true);
      ++length;
    }
  }

  return length;
}

///
/// \param[in] ind individual to be analyzed.
/// \return effective length of individual we gathered statistics about.
///
unsigned analyzer<i_ga>::count(const i_ga &ind)
{
  for (const auto &g : ind)
    core_analyzer<i_ga>::count(g.sym, true);

  return ind.parameters();
}

///
/// \param[in] ind individual to be analyzed.
/// \return effective length of individual we gathered statistics about.
///
unsigned analyzer<i_de>::count(const i_de &ind)
{
  return ind.parameters();
}

#endif  // include guard
