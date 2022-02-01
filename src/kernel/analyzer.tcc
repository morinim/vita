/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2022 EOS di Manlio Morini.
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
/// New empty analyzer.
///
template<class T>
analyzer<T>::analyzer()
{
  clear();
}

///
/// Resets gathered statics.
///
template<class T>
void analyzer<T>::clear()
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
///         informations about
///
template<class T>
typename analyzer<T>::const_iterator analyzer<T>::begin() const
{
  return sym_counter_.begin();
}

///
/// \return a constant reference (sentry) used for loops
///
template<class T>
typename analyzer<T>::const_iterator analyzer<T>::end() const
{
  return sym_counter_.end();
}

///
/// \param[in] eff effective / noneffective functions
/// \return        number of functions in the population
///
template<class T>
std::uintmax_t analyzer<T>::functions(bool eff) const
{
  return functions_.counter[eff];
}

///
/// \param[in] eff effective / noneffective terminals
/// \return        number of terminals in the population
///
template<class T>
std::uintmax_t analyzer<T>::terminals(bool eff) const
{
  return terminals_.counter[eff];
}

///
/// \return statistics about the age distribution of the individuals
///
template<class T>
const distribution<double> &analyzer<T>::age_dist() const
{
  return age_;
}

///
/// \param[in] g a group
/// \return      statistics about the age distribution of individuals in group
///              `g`
///
template<class T>
const distribution<double> &analyzer<T>::age_dist(unsigned g) const
{
  const auto gi(group_stat_.find(g));
  assert(gi != group_stat_.end());

  return gi->second.age;
}

///
/// \return statistics about the fitness distribution of the individuals
///
template<class T>
const distribution<fitness_t> &analyzer<T>::fit_dist() const
{
  return fit_;
}

///
/// \param[in] g a group
/// \return      statistics about the fitness distribution of individuals in
///              group `g`
///
template<class T>
const distribution<fitness_t> &analyzer<T>::fit_dist(unsigned g) const
{
  const auto gi(group_stat_.find(g));
  assert(gi != group_stat_.end());

  return gi->second.fitness;
}

///
/// \return statistic about the length distribution of the individuals
///
template<class T>
const distribution<double> &analyzer<T>::length_dist() const
{
  return length_;
}

///
/// \param[in] sym    symbol we are gathering statistics about
/// \param[in] active is this an active gene?
///
/// Used by `count(const T &)`.
///
template<class T>
void analyzer<T>::count(const symbol *sym, bool active)
{
  Expects(sym);

  ++sym_counter_[sym].counter[active];

  if (sym->terminal())
    ++terminals_.counter[active];
  else
    ++functions_.counter[active];
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T>
bool analyzer<T>::is_valid() const
{
  return std::all_of(sym_counter_.begin(), sym_counter_.end(),
                     [](const auto &e)
                     {
                       return e.second.counter[true] <= e.second.counter[false];
                     });
}

///
/// Adds a new individual to the pool used to calculate statistics.
///
/// \param[in] ind new individual
/// \param[in] f   fitness of the new individual
/// \param[in] g   a group of the population
///
/// The optional `g` parameter can be used to group information (e.g. for the
/// ALPS algorithm it's used for layer specific statistics).
///
template<class T>
void analyzer<T>::add(const T &ind, const fitness_t &f, unsigned g)
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
/// \tparam T type of individual
///
/// \param[in] ind individual to be analyzed
/// \return        effective length of individual we gathered statistics about
///
template<class T>
std::size_t analyzer<T>::count(const T &ind)
{
  return count_team(ind, is_team<T>());
}

///
/// Specialization of `count_team(T)` for non-team.
///
template<class T>
template<class U>
std::size_t analyzer<T>::count_team(const U &ind, std::false_type)
{
  return count_introns(ind, has_introns<T>());
}

///
/// Specialization of `count_team(T)` for teams.
///
template<class T>
std::size_t analyzer<T>::count_team(const T &t, std::true_type)
{
  return std::accumulate(t.begin(), t.end(), std::size_t{0},
                         [=](auto s, const auto &ind)
                         {
                           return s + count_team(ind, std::false_type());
                         });
}

///
/// Specialization of `count_introns(U)` for individuals with introns.
///
template<class T>
template<class U>
std::size_t analyzer<T>::count_introns(const U &ind, std::true_type)
{
  for (index_t i(0); i < ind.size(); ++i)
    for (category_t c(0); c < ind.categories(); ++c)
      count(ind[{i, c}].sym, false);

  return count_introns(ind, std::false_type());
}

///
/// Specialization of `count_introns(U)` for individuals without introns.
///
template<class T>
template<class U>
std::size_t analyzer<T>::count_introns(const U &ind, std::false_type)
{
  std::size_t length(0);
  for (const auto &g : ind)
  {
    count(g.sym, true);
    ++length;
  }

  return length;
}

///
/// \param[in] ind individual to be analyzed
/// \return        effective length of individual we gathered statistics about
///
template<>
inline std::size_t analyzer<i_de>::count(const i_de &ind)
{
  return ind.parameters();
}

///
/// \param[in] ind individual to be analyzed
/// \return        effective length of individual we gathered statistics about
///
template<>
inline std::size_t analyzer<i_ga>::count(const i_ga &ind)
{
  return ind.parameters();
}

#endif  // include guard
