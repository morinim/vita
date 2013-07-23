/**
 *
 *  \file team_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(TEAM_INL_H)
#define      TEAM_INL_H

///
/// \param[in] e base environment.
/// \param[in] sset a symbol set.
///
/// Creates a team of individuals that will cooperate to solve a task.
///
template<class T>
team<T>::team(const environment &e, const symbol_set &sset) : signature_()
{
  assert(e.debug(true, true));
  assert(e.team.size);

  individuals_.reserve(e.team.size);
  for (unsigned i(0); i < e.team.size; ++i)
    individuals_.emplace_back(e, sset);

  assert(debug());
}

///
/// \return number of mutations performed.
///
/// Calls team::mutation(double) using the mutation probability specified in
/// the environment.
///
template<class T>
unsigned team<T>::mutation()
{
  assert(env().p_mutation);
  return mutation(*env().p_mutation);
}

///
/// \param[in] p probability of gene mutation.
/// \return number of mutations performed.
///
/// A new team is created mutating \c this.
///
template<class T>
unsigned team<T>::mutation(double p)
{
  unsigned n(0);
  for (auto &i : individuals_)
    n += i.mutation(p);

  assert(debug());
  return n;
}

///
/// \return an iterator pointing to the first individual of the team.
///
template<class T>
typename team<T>::const_iterator team<T>::begin() const
{
  return individuals_.begin();
}

///
/// \return an iterator pointing to a end-of-team sentry.
///
template<class T>
typename team<T>::const_iterator team<T>::end() const
{
  return individuals_.end();
}

///
/// \param[in] l locus of a \c gene.
/// \return the l-th \c gene of \a this \c individual.
///
template<class T>
const T &team<T>::operator[](unsigned i) const
{
  assert(i < individuals_.size());
  return individuals_[i];
}

///
/// \return the signature of \c this team.
///
/// Signature maps syntactically distinct (but logically equivalent)
/// teams to the same value.
///
/// In other words identical teamss at genotypic level have the same
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
/// \return the signature of \c this team.
///
///
template<class T>
hash_t team<T>::hash() const
{
  hash_t ret;

  for (const auto &i : individuals_)
    ret ^= i.signature();

  return ret;
}

///
/// \param[in] x second term of comparison.
/// \return \c true if the two teams are equal (individual by individual).
///
/// \note
/// Age is not checked.
///
template<class T>
bool team<T>::operator==(const team<T> &x) const
{
  const unsigned sup(individuals_.size());
  for (unsigned i(0); i < sup; ++i)
    if (!(individuals_[i] == x[i]))
      return false;

  return true;
}

///
/// \return the age of the team (average age of the team members).
///
template<class T>
unsigned team<T>::age() const
{
  unsigned age(0);
  for (const auto &i: individuals_)
    age += i.age();

  return age / individuals_.size();
}

///
/// \return the environment of the team.
///
template<class T>
const environment &team<T>::env() const
{
  return individuals_[0].env();
}

///
/// \return the symbol_set of the team.
///
template<class T>
const symbol_set &team<T>::sset() const
{
  return individuals_[0].sset();
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the team passes the internal consistency check.
///
template<class T>
bool team<T>::debug(bool verbose) const
{
  for (const auto &i : individuals_)
    if (!i.debug(verbose))
      return false;

  if (!signature_.empty() && signature_ != hash())
    return false;

  return env().debug(verbose, true);
}

///
/// \param[out] s output stream.
/// \param[in] ind individual to print.
/// \return output stream including \a ind.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const team<T> &t)
{
  for (const auto &i : t)
    s << i << std::endl;
  return s;
}
#endif  // TEAM_INL_H
