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
basic_team<T>::basic_team(const environment &e, const symbol_set &sset)
  : signature_()
{
  assert(e.debug(true, true));
  assert(e.team.individuals);

  const auto n(e.team.individuals);
  individuals_.reserve(n);

  for (decltype(e.team.individuals) i(0); i < n; ++i)
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
unsigned basic_team<T>::mutation()
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
unsigned basic_team<T>::mutation(double p)
{
  return random::element(individuals_).mutation(p);
}

///
/// \param[in] p the second parent.
/// \return the result of the crossover (we only generate a single
///         offspring).
///
/// \see individual::crossover for further details
///
template<class T>
basic_team<T> basic_team<T>::crossover(const basic_team<T> &p) const
{
  assert(p.debug());
  assert(size() == p.size());

  const auto j(random::sup(individuals()));

  basic_team<T> offspring(*this);
  offspring.individuals_[j] = offspring[j].crossover(p[j]);

  return offspring;
}

///
/// \return an iterator pointing to the first individual of the team.
///
template<class T>
typename basic_team<T>::const_iterator basic_team<T>::begin() const
{
  return individuals_.begin();
}

///
/// \return an iterator pointing to a end-of-team sentry.
///
template<class T>
typename basic_team<T>::const_iterator basic_team<T>::end() const
{
  return individuals_.end();
}

///
/// \param[in] l locus of a \c gene.
/// \return the l-th \c gene of \a this \c individual.
///
template<class T>
const T &basic_team<T>::operator[](unsigned i) const
{
  assert(i < individuals());
  return individuals_[i];
}

///
/// \return number of individuals of the team.
///
template<class T>
unsigned basic_team<T>::individuals() const
{
  return individuals_.size();
}

///
/// \return the total size of the team (effective size + introns).
///
/// The size is constant for any team (it's choosen at initialization time).
/// \see eff_size()
///
template<class T>
unsigned basic_team<T>::size() const
{
  unsigned s(0);

  for (const auto &i : individuals_)
    s += i.size();

  return s;
}

///
/// \return the effective size of the team.
/// \see size()
///
template<class T>
unsigned basic_team<T>::eff_size() const
{
  unsigned ef(0);

  for (const auto &ind : individuals_)
    for (typename T::const_iterator it(ind); it(); ++it)
      ++ef;

  return ef;
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
hash_t basic_team<T>::signature() const
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
hash_t basic_team<T>::hash() const
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
bool basic_team<T>::operator==(const basic_team<T> &x) const
{
  const auto sup(individuals());
  for (unsigned i(0); i < sup; ++i)
    if (!(individuals_[i] == x[i]))
      return false;

  return true;
}

///
/// \param[in] x a team to compare with \c this.
/// \return a numeric measurement of the difference between \a x and \c this
/// (the number of different genes between teams).
///
template<class T>
unsigned basic_team<T>::distance(const basic_team<T> &x) const
{
  unsigned d(0);

  const auto sup(individuals());
  for (unsigned i(0); i < sup; ++i)
  {
    const index_t cs(individuals_[i].size());
    const category_t categories(sset().categories());

    for (index_t j(0); j < cs; ++j)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{j, c};
        if (individuals_[i][l] != x[i][l])
          ++d;
      }
  }

  return d;
}

///
/// \return the age of the team (average age of the team members).
///
template<class T>
unsigned basic_team<T>::age() const
{
  unsigned age(0);
  for (const auto &i : individuals_)
    age += i.age();

  return age / individuals();
}

///
/// Increments the age of every element of the team.
///
template<class T>
void basic_team<T>::inc_age()
{
  for (auto &i : individuals_)
    i.inc_age();
}

///
/// \return the environment of the team.
///
template<class T>
const environment &basic_team<T>::env() const
{
  return individuals_[0].env();
}

///
/// \return the symbol_set of the team.
///
template<class T>
const symbol_set &basic_team<T>::sset() const
{
  return individuals_[0].sset();
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the team passes the internal consistency check.
///
template<class T>
bool basic_team<T>::debug(bool verbose) const
{
  for (const auto &i : individuals_)
    if (!i.debug(verbose))
      return false;

  if (!signature_.empty() && signature_ != hash())
    return false;

  return env().debug(verbose, true);
}

///
/// \param[in] in input stream.
/// \return \c true if team was loaded correctly.
///
/// \note
/// If the load operation isn't successful the current team isn't modified.
///
template<class T>
bool basic_team<T>::load(std::istream &in)
{
  unsigned n;
  if (!(in >> n))
    return false;

  decltype(individuals_) v;
  v.reserve(n);

  for (unsigned j(0); j < n; ++j)
  {
    T i(env(), sset());
    if (!i.load(in))
      return false;
    v.push_back(i);
  }

  individuals_ = v;

  // We don't save/load signature: it can be easily calculated on the fly.
  signature_.clear();
  // signature_ = hash();

  return true;
}

///
/// \param[out] out output stream.
/// \return \c true if team was saved correctly.
///
template<class T>
bool basic_team<T>::save(std::ostream &out) const
{
  out << individuals() << std::endl;
  if (!out.good())
    return false;

  for (const auto &i : individuals_)
    if (!i.save(out))
      return false;

  return true;
}

///
/// \param[out] s output stream.
/// \param[in] ind individual to print.
/// \return output stream including \a ind.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const basic_team<T> &t)
{
  for (const auto &i : t)
    s << i << std::endl;
  return s;
}
#endif  // TEAM_INL_H
