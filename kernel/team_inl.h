/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_TEAM_INL_H)
#define      VITA_TEAM_INL_H

///
/// \param[in] e base environment.
/// \param[in] ss a symbol set.
///
/// Creates a team of individuals that will cooperate to solve a task.
///
template<class T>
team<T>::team(const environment &e, const symbol_set &ss) : signature_()
{
  assert(e.debug(true, true));
  assert(e.team.individuals);

  const auto n(e.team.individuals);
  individuals_.reserve(n);

  for (auto i(decltype(n){0}); i < n; ++i)
    individuals_.emplace_back(e, ss);

  assert(debug());
}

///
/// \param v a vector of individuals
///
/// Builds a team containing the individuals of vector \a v.
///
template<class T>
team<T>::team(std::vector<T> v) : individuals_(std::move(v)), signature_()
{
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
  const auto p(env().p_mutation);
  assert(0 <= p && p <= 1.0);

  return mutation(p);
}

///
/// \param[in] p probability of gene mutation.
/// \return number of mutations performed.
///
/// Mutates a random individual in \c this team and returns the resulting new
/// team.
///
template<class T>
unsigned team<T>::mutation(double p)
{
  assert(0.0 <= p && p <= 1.0);

  /*
  const auto nm(random::element(individuals_).mutation(p));
  if (nm)
    signature_.clear();

  return nm;
  */

  unsigned nm(0);
  for (auto &i : individuals_)
    nm += i.mutation(p);

  if (nm)
    signature_.clear();

  return nm;
}

///
/// \param[in] t the second parent.
/// \return the result of the crossover (we only generate a single
///         offspring).
///
/// \see individual::crossover for further details
///
template<class T>
team<T> team<T>::crossover(team<T> t) const
{
  assert(t.debug());
  assert(individuals() == t.individuals());

/*
  const auto j(random::sup(individuals()));

  t.individuals_[j] = operator[](j).crossover(t[j])
  t.signature_.clear();
*/

  const auto sup(individuals());
  for (auto i(decltype(sup){0}); i < sup; ++i)
    t.individuals_[i] = operator[](i).crossover(t[i]);

  t.signature_.clear();
  return t;
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
/// \param[in] i index of a member of the team.
/// \return the i-th member of the team.
///
template<class T>
const T &team<T>::operator[](unsigned i) const
{
  assert(i < individuals());
  return individuals_[i];
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
/// \return the effective size of the team.
/// \see team::size()
///
template<class T>
unsigned team<T>::eff_size() const
{
  unsigned es(0);

  for (const auto &ind : individuals_)
    es += ind.eff_size();

  return es;
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
    ret.combine(i.signature());

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
  const auto sup(individuals());
  for (auto i(decltype(sup){0}); i < sup; ++i)
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
unsigned team<T>::distance(const team<T> &x) const
{
  unsigned d(0);

  const auto sup(individuals());
  for (auto i(decltype(sup){0}); i < sup; ++i)
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
unsigned team<T>::age() const
{
  unsigned age_sum(0);
  for (const auto &i : individuals_)
    age_sum += i.age();

  return age_sum / individuals();
}

///
/// Increments the age of every element of the team.
///
template<class T>
void team<T>::inc_age()
{
  for (auto &i : individuals_)
    i.inc_age();
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
/// \param[in] in input stream.
/// \return \c true if team was loaded correctly.
///
/// \note
/// If the load operation isn't successful the current team isn't modified.
///
template<class T>
bool team<T>::load(std::istream &in)
{
  unsigned n;
  if (!(in >> n) || !n)
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

  return true;
}

///
/// \param[out] out output stream.
/// \return \c true if team was saved correctly.
///
template<class T>
bool team<T>::save(std::ostream &out) const
{
  out << individuals() << std::endl;
  if (!out.good())
    return false;

  for (const auto &i : individuals_)
    if (!i.save(out))
      return false;

  return out.good();
}

///
/// \param[out] s output stream.
/// \param[in] t team to print.
/// \return output stream including \a t.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const team<T> &t)
{
  for (const auto &i : t)
    s << i << std::endl;
  return s;
}

///
/// \param[out] s output stream.
///
/// The output stream contains a graph, described in dot language
/// (http://www.graphviz.org), of \c this team.
///
template<class T>
void team<T>::graphviz(std::ostream &s) const
{
  const auto size(individuals_.size());

  s << "graph {";
  for (auto i(decltype(size){0}); i < size; ++i)
    individuals_[i].graphviz(s,
                             std::string("Individual ") +
                             std::to_string(i));

  s << '}' << std::endl;
}

///
/// \param[out] s output stream
///
/// The \a team is printed on a single line with symbols separated by
/// spaces and individuals between curly braces.
/// Not at all human readable, but a compact representation for import/export.
///
template<class T>
void team<T>::in_line(std::ostream &s) const
{
  for (const auto &i : individuals_)
  {
    s << '{';
    i.in_line(s);
    s << '}';
  }
}

///
/// \param[out] s output stream
///
/// Do you remember C=64 list? :-)
///
/// 10 PRINT "HOME"
/// 20 PRINT "SWEET"
/// 30 GOTO 10
///
template<class T>
void team<T>::list(std::ostream &s) const
{
  for (const auto &i : individuals_)
  {
    i.list(s);
    s << std::endl;
  }
}

///
/// \param[out] s output stream.
///
template<class T>
void team<T>::tree(std::ostream &s) const
{
  for (const auto &i : individuals_)
  {
    i.tree(s);
    s << std::endl;
  }
}
#endif  // Include guard
