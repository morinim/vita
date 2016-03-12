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

#if !defined(VITA_TEAM_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_TEAM_TCC)
#define      VITA_TEAM_TCC

template<class T>
team<T>::team() : individuals_(), signature_()
{
}

///
/// \param[in] e base environment.
///
/// Creates a team of individuals that will cooperate to solve a task.
///
template<class T>
team<T>::team(const environment &e) : signature_()
{
  assert(e.debug(true));
  assert(e.team.individuals);
  assert(e.sset);

  const auto n(e.team.individuals);
  individuals_.reserve(n);

  for (auto i(decltype(n){0}); i < n; ++i)
    individuals_.emplace_back(e);

  assert(debug());
}

///
/// \param v a vector of individuals
///
/// Builds a team containing the individuals of vector `v`.
///
template<class T>
team<T>::team(std::vector<T> v) : individuals_(std::move(v)), signature_()
{
  assert(debug());
}

///
/// \param[in] p probability of gene mutation.
/// \param[in] env the current environment.
/// \return number of mutations performed.
///
/// Mutates the individuals in `this` team and returns the number of mutations
/// performed.
///
template<class T>
unsigned team<T>::mutation(double p, const environment &env)
{
  Expects(0.0 <= p);
  Expects(p <= 1.0);

  /*
  const auto nm(random::element(individuals_).mutation(p, sset));
  if (nm)
    signature_.clear();

  return nm;
  */

  unsigned nm(0);
  for (auto &i : individuals_)
    nm += i.mutation(p, env);

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
/// \return `true` if the team is empty, `false` otherwise.
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
/// \return the signature of `this` team.
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
/// \return the signature of `this` team.
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
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
/// \return `true` if the two teams are equal (individual by individual).
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

  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
/// \return a numeric measurement of the difference between `x` and `this`
///         (the number of different genes between teams).
///
/// \relates team<T>
///
template<class T>
unsigned distance(const team<T> &lhs, const team<T> &rhs)
{
  const auto sup(lhs.individuals());
  assert(sup == rhs.individuals());

  unsigned d(0);
  for (auto i(decltype(sup){0}); i < sup; ++i)
    d += distance(lhs[i], rhs[i]);

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
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the team passes the internal consistency check.
///
template<class T>
bool team<T>::debug() const
{
  for (const auto &i : individuals_)
    if (!i.debug())
      return false;

  return signature_.empty() || signature_ == hash();
}

///
/// \param[in] e environment used to build the individual.
/// \param[in] in input stream.
/// \return `true` if team was loaded correctly.
///
/// \note
/// If the load operation isn't successful the current team isn't modified.
///
template<class T>
bool team<T>::load(std::istream &in, const environment &e)
{
  unsigned n;
  if (!(in >> n) || !n)
    return false;

  decltype(individuals_) v;
  v.reserve(n);

  for (unsigned j(0); j < n; ++j)
  {
    T i;
    if (!i.load(in, e))
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
/// \return `true` if team was saved correctly.
///
template<class T>
bool team<T>::save(std::ostream &out) const
{
  out << individuals() << '\n';
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
/// \return output stream including `t`.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const team<T> &t)
{
  for (const auto &i : t)
    s << i << '\n';
  return s;
}

///
/// \param[out] s output stream.
///
/// The output stream contains a graph, described in dot language
/// (http://www.graphviz.org), of `this` team.
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

  s << "}\n";
}

///
/// \param[out] s output stream
///
/// The team is printed on a single line with symbols separated by
/// spaces and individuals between curly braces.
/// Not at all human readable, but a compact representation for import/export.
///
template<class T>
std::ostream &team<T>::in_line(std::ostream &s) const
{
  for (const auto &i : individuals_)
  {
    s << '{';
    i.in_line(s);
    s << '}';
  }

  return s;
}

///
/// \param[out] s output stream
/// \param[in] short_form if `true` prints a shorter and more human-readable
///                       form of the genome.
///
/// Do you remember C=64 list? :-)
///
/// 10 PRINT "HOME"
/// 20 PRINT "SWEET"
/// 30 GOTO 10
///
template<class T>
std::ostream &team<T>::list(std::ostream &s, bool short_form) const
{
  for (const auto &i : individuals_)
  {
    i.list(s, short_form);
    s << '\n';
  }

  return s;
}

///
/// \param[out] s output stream.
///
template<class T>
std::ostream &team<T>::tree(std::ostream &s) const
{
  for (const auto &i : individuals_)
  {
    i.tree(s);
    s << '\n';
  }

  return s;
}
#endif  // Include guard
