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

#if !defined(VITA_TEAM_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_TEAM_TCC)
#define      VITA_TEAM_TCC

template<class T>
team<T>::team() : individuals_(), signature_()
{
}

template<class T>
team<T>::team(unsigned n) : individuals_(n), signature_()
{
}

///
/// Creates a team of individuals that will cooperate to solve a task.
///
/// \param[in] p current problem
///
template<class T>
team<T>::team(const problem &p) : signature_()
{
  Expects(p.env.team.individuals);

  auto n(p.env.team.individuals);
  individuals_.reserve(n);

  for (decltype(n) i(0); i < n; ++i)
    individuals_.emplace_back(p);

  Ensures(is_valid());
}

///
/// Builds a team containing the individuals of a given vector.
///
/// \param[in] v a vector of individuals
///
template<class T>
team<T>::team(std::vector<T> v) : individuals_(std::move(v)), signature_()
{
  Ensures(is_valid());
}

///
/// Mutates the individuals in `this` team and returns the number of mutations
/// performed.
///
/// \param[in] pgm probability of gene mutation
/// \param[in] prb current problem
/// \return        number of mutations performed
///
template<class T>
unsigned team<T>::mutation(double pgm, const problem &prb)
{
  Expects(0.0 <= pgm);
  Expects(pgm <= 1.0);

  /*
  const auto nm(random::element(individuals_).mutation(pgm, prb));
  if (nm)
    signature_.clear();

  return nm;
  */

  unsigned nm(0);
  for (auto &i : individuals_)
    nm += i.mutation(pgm, prb);

  if (nm)
    signature_.clear();

  return nm;
}

///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        the result of the crossover (we only generate a single
///                offspring)
///
/// \see individual::crossover for further details.
///
template<class T>
team<T> crossover(const team<T> &lhs, const team<T> &rhs)
{
  Expects(lhs.individuals() == rhs.individuals());

/*
  const auto j(random::sup(lhs.individuals()));

  auto ret(lhs);
  ret.individuals_[j] = crossover(lhs[j], rhs[j])
  ret.signature_.clear();
*/

  const auto sup(lhs.individuals());
  team<T> ret(sup);

  for (unsigned i(0); i < sup; ++i)
    ret.individuals_[i] = crossover(lhs[i], rhs[i]);

  // Clearing signature isn't required.

  Ensures(ret.is_valid());
  return ret;
}

///
/// \return an iterator pointing to the first individual of the team
///
template<class T>
typename team<T>::const_iterator team<T>::begin() const
{
  return individuals_.begin();
}

///
/// \return an iterator pointing to a end-of-team sentry
///
template<class T>
typename team<T>::const_iterator team<T>::end() const
{
  return individuals_.end();
}

///
/// \param[in] i index of a member of the team
/// \return      the `i`-th member of the team
///
template<class T>
const T &team<T>::operator[](unsigned i) const
{
  Expects(i < individuals());
  return individuals_[i];
}

///
/// \return `true` if the team is empty, `false` otherwise
///
template<class T>
bool team<T>::empty() const
{
  return individuals_.empty();
}

///
/// \return number of individuals of the team.
///
template<class T>
unsigned team<T>::individuals() const
{
  return static_cast<unsigned>(individuals_.size());
}

///
/// \return the number of active symbols in the team
/// \see team::size()
///
template<class T>
unsigned team<T>::active_symbols() const
{
  return std::accumulate(begin(), end(), 0,
                         [](unsigned n, const T &ind)
                         {
                           return n + ind.active_symbols();
                         });
}

///
/// Signature maps syntactically distinct (but logically equivalent)
/// teams to the same value.
///
/// \return the signature of `this` team
///
/// In other words identical teams, at genotypic level, have the same
/// signature; different teams at the genotipic level may be mapped
/// to the same signature since the value of terminals is considered and not
/// the index.
///
/// This is a very interesting  property, useful for comparison, information
/// retrieval, entropy calculation...
///
template<class T>
hash_t team<T>::signature() const
{
  if (signature_.empty())
    signature_ = hash();

  return signature_;
}

///
/// \return the signature of `this` team
///
///
template<class T>
hash_t team<T>::hash() const
{
  hash_t ret;

  for_each(begin(), end(),
           [&ret](const T &i) { ret.combine(i.signature()); });

  return ret;
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparision
/// \return        `true` if the two teams are equal (individual by individual)
///
/// \note Age is not checked.
///
/// \relates team<T>
///
template<class T>
bool operator==(const team<T> &lhs, const team<T> &rhs)
{
  const auto sup(lhs.individuals());
  if (sup != rhs.individuals())
    return false;

  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparision
/// \return        `true` if the two teams aren't equal
///
/// \relates team<T>
///
template<class T>
bool operator!=(const team<T> &lhs, const team<T> &rhs)
{
  return !(lhs == rhs);
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparision
/// \return        a numeric measurement of the difference between `x` and
///                `this` (the number of different genes between teams)
///
/// \relates team<T>
///
template<class T>
unsigned distance(const team<T> &lhs, const team<T> &rhs)
{
  Expects(lhs.individuals() == rhs.individuals());

  const auto sup(lhs.individuals());
  unsigned d(0);
  for (auto i(decltype(sup){0}); i < sup; ++i)
    d += distance(lhs[i], rhs[i]);

  return d;
}

///
/// \return the age of the team (average age of the team members)
///
template<class T>
unsigned team<T>::age() const
{
  const unsigned age_sum(std::accumulate(begin(), end(), 0,
                                         [](unsigned sum, const T &i)
                                         {
                                           return sum + i.age();
                                         }));

  return age_sum / individuals();
}

///
/// Increments the age of every element of the team
///
template<class T>
void team<T>::inc_age()
{
  for (auto &i : individuals_)
    i.inc_age();
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`
/// \return            `true` if the team passes the internal consistency check
///
template<class T>
bool team<T>::is_valid() const
{
  return signature_.empty() || signature_ == hash();
}

///
/// \param[in] ss active symbol set
/// \param[in] in input stream
/// \return       `true` if team was loaded correctly
///
/// \note
/// If the load operation isn't successful the current team isn't modified.
///
template<class T>
bool team<T>::load(std::istream &in, const symbol_set &ss)
{
  unsigned n;
  if (!(in >> n) || !n)
    return false;

  decltype(individuals_) v;
  v.reserve(n);

  for (unsigned j(0); j < n; ++j)
  {
    T i;
    if (!i.load(in, ss))
      return false;
    v.push_back(i);
  }

  individuals_ = v;

  // We don't save/load signature: it can be easily calculated on the fly.
  signature_.clear();

  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if team was saved correctly
///
template<class T>
bool team<T>::save(std::ostream &out) const
{
  out << individuals() << '\n';
  if (!out.good())
    return false;

  if (std::any_of(begin(), end(),
                  [&](const T &i) { return !i.save(out); }))
    return false;

  return out.good();
}

///
/// \param[out] s output stream
/// \param[in]  t team to print
/// \return       output stream including `t`
///
/// \relates team<T>
///
template<class T>
std::ostream &operator<<(std::ostream &s, const team<T> &t)
{
  const auto format(out::print_format_flag(s));

  for (const auto &i : t)
  {
    if (format == out::in_line_f)
      s << '{';

    s << i;

    if (format == out::in_line_f)
      s << '}';
    else
      s << '\n';
  }

  return s;
}

#endif  // include guard
