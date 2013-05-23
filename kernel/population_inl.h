/**
 *
 *  \file population_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(POPULATION_INL_H)
#define      POPULATION_INL_H

///
/// \param[in] i index of an \a individual.
/// \return a reference to the \a individual at index \a i.
///
template<class T>
T &population<T>::operator[](size_t i)
{
  assert(i < individuals());
return pop_[i];
}

///
/// \param[in] i index of an individual.
/// \return a constant reference to the individual at index \a i.
///
template<class T>
const T &population<T>::operator[](size_t i) const
{
  assert(i < individuals());
return pop_[i];
}

///
/// \return the number of individuals in the population.
///
template<class T>
size_t population<T>::individuals() const
{
  return pop_.size();
}

///
/// \return a constant reference to the active environment.
///
template<class T>
const environment &population<T>::env() const
{
  return pop_[0].env();
}

///
/// \return an iterator pointing to the first individual of the population.
///
template<class T>
typename population<T>::const_iterator population<T>::begin() const
{
  return pop_.begin();
}

///
/// \return an itarator pointing one element past the last individual of the
///         population
///
template<class T>
typename population<T>::const_iterator population<T>::end() const
{
  return pop_.end();
}

///
/// \param[in] e base vita::environment.
///
/// Creates a random population (initial size \a e.individuals).
///
template<class T>
population<T>::population(const environment &e)
{
  assert(e.debug(true, true));

  pop_.reserve(e.individuals);
  pop_.clear();

  for (size_t i(0); i < e.individuals; ++i)
    pop_.emplace_back(e, true);

  assert(debug(true));
}

///
/// Increments the age of each individual in the population.
///
template<class T>
void population<T>::inc_age()
{
  for (auto &i : pop_)
    ++i.age;
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool population<T>::debug(bool verbose) const
{
  for (const auto &i : pop_)
    if (!i.debug(verbose))
      return false;

  return true;
}

///
/// \param[in] in input stream.
/// \return \c true if population was loaded correctly.
///
/// \note
/// If the load operation isn't successful the current population isn't
/// changed.
///
template<class T>
bool population<T>::load(std::istream &in)
{
  size_t n_elem(0);
  if (!(in >> n_elem))
    return false;

  population p(env());
  for (size_t i(0); i < n_elem; ++i)
    if (!p[i].load(in))
      return false;

  *this = p;
return true;
}

///
/// \param[out] out output stream.
/// \return \c true if population was saved correctly.
///
template<class T>
bool population<T>::save(std::ostream &out) const
{
  out << individuals() << std::endl;

  for (const auto &i : *this)
    i.save(out);

  return out.good();
}

///
/// \param[in,out] s output \c stream.
/// \param[in] pop population to be listed.
/// \return the output \c stream.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const population<T> &pop)
{
  for (const auto &i : pop)
    s << i << std::endl;

  return s;
}
#endif  // POPULATION_INL_H
