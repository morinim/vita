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

#if !defined(VITA_POPULATION_INL_H)
#define      VITA_POPULATION_INL_H

///
/// \param[in] e base vita::environment.
/// \param[in] sset base vita::symbol_set.
///
/// Creates a random population (initial size \a e.individuals).
///
template<class T>
population<T>::population(const environment &e, const symbol_set &sset)
  : pop_(1), allowed_(1)
{
  assert(e.debug(true, true));

  const auto n(e.individuals);
  pop_[0].reserve(n);
  allowed_[0] = n;

  // DO NOT CHANGE with a call to init_layer(0): when layer 0 is empty, there
  // isn't a well defined environment and init_layer doesn't work.
  for (auto i(decltype(n){0}); i < n; ++i)
    pop_[0].emplace_back(e, sset);

  assert(debug(true));
}

///
/// \param[in] l a layer of the population.
/// \param[in] e an environmnet (used for individual generation).
/// \param[in] s a symbol_set (used for individual generation).
///
/// Resets layer \a l of the population.
///
/// \warning
/// If layer \a l is nonexistent/empty the method doesn't work!
///
template<class T>
void population<T>::init_layer(unsigned l, const environment *e,
                               const symbol_set *s)
{
  assert(l < layers());
  assert(individuals(l) || (e && s));

  if (!e)
    e = &pop_[l][0].env();
  if (!s)
    s = &pop_[l][0].sset();

  pop_[l].clear();

  const auto n(allowed(l));
  for (auto i(decltype(n){0}); i < n; ++i)
    pop_[l].emplace_back(*e, *s);
}

///
/// \return number of active layers.
///
/// \note
/// * The number of active layers is a dynamic value (it is a monotonically
///   increasing function of the generation number).
/// * Maximum number of layers (\c env().alps.layers) is a constant value
///   greater than or equal to \c layers().
///
template<class T>
unsigned population<T>::layers() const
{
  return pop_.size();
}

///
/// Add a new layer to the population.
///
/// The new layer is inserted as the lower layer and randomly initialized.
///
template<class T>
void population<T>::add_layer()
{
  assert(layers());
  assert(individuals(0));

  const auto &e(env());
  const auto &s(pop_[0][0].sset());

  pop_.insert(pop_.begin(), layer_t());
  pop_[0].reserve(e.individuals);

  allowed_.insert(allowed_.begin(), e.individuals);

  init_layer(0, &e, &s);
}

///
/// \param[in] l index of a layer.
/// \param[in] i an individual.
///
/// Add individual \a i to layer \a l.
///
template<class T>
void population<T>::add_to_layer(unsigned l, const T &i)
{
  assert(l < layers());

  if (individuals(l) < allowed(l))
    pop_[l].push_back(i);
}

///
/// \param[in] l index of a layer.
///
/// Remove the last individual of layer \a l.
///
template<class T>
void population<T>::pop_from_layer(unsigned l)
{
  assert(l < layers());
  pop_[l].pop_back();
}


///
/// \param[in] c coordinates of an \a individual.
/// \return a reference to the \a individual at coordinates \a c.
///
template<class T>
T &population<T>::operator[](coord c)
{
  assert(c.layer < layers());
  assert(c.index < individuals(c.layer));
  return pop_[c.layer][c.index];
}

///
/// \param[in] c coordinates of an individual.
/// \return a constant reference to the individual at coordinates \a c.
///
template<class T>
const T &population<T>::operator[](coord c) const
{
  assert(c.layer < layers());
  assert(c.index < individuals(c.layer));
  return pop_[c.layer][c.index];
}

///
/// \param[in] l a layer.
/// \return the number of individuals allowed in layer \a l.
///
/// \note
/// for each l: individuals(l) < allowed(l)
///
template<class T>
unsigned population<T>::allowed(unsigned l) const
{
  assert(l < layers());
  return allowed_[l];
}

///
/// \param[in] l a layer.
/// \param[in] n number of programs allowed in layer \a l.
///
/// Sets the number of programs allowed in layer \a l. If layer \a l contains
/// more programs than the allowed, the excedence will be deleted.
///
template<class T>
void population<T>::set_allowed(unsigned l, unsigned n)
{
  assert(l < layers());
  assert(n <= pop_[l].capacity());

  if (individuals(l) > n)
  {
    unsigned delta(individuals(l) - n);

    while (delta)
    {
      pop_[l].pop_back();
      --delta;
    }
  }

  assert(individuals(l) == n);
  allowed_[l] = n;

  assert(debug(true));
}

///
/// \param[in] l a layer.
/// \return the number of individuals in layer \a l.
///
template<class T>
unsigned population<T>::individuals(unsigned l) const
{
  assert(l < layers());
  return pop_[l].size();
}

///
/// \return the number of individuals in the population.
///
template<class T>
unsigned population<T>::individuals() const
{
  unsigned n(0);
  for (const auto &layer : pop_)
    n += layer.size();

  return n;
}

///
/// \return a constant reference to the active environment.
///
template<class T>
const environment &population<T>::env() const
{
  assert(layers());
  assert(individuals(0));

  return pop_[0][0].env();
}

///
/// \return a const_iterator pointing to the first layer of the population.
///
/// \note
/// There isn't a non const version of this method. This is a precise choice:
/// begin() can sometimes be a fast method to access the population (i.e. when
/// we work a layer at time) but it cannot be a way of changing elements of the
/// population without breaking class encapsulation.
///
/// \warning
/// Pointer to the first LAYER *NOT* to the first PROGRAM.
///
template<class T>
typename population<T>::const_iterator population<T>::begin() const
{
  return pop_.begin();
}

///
/// \return an iterator pointing one element past the last individual of the
///         population
///
template<class T>
typename population<T>::const_iterator population<T>::end() const
{
  return pop_.end();
}

///
/// Increments the age of each individual in the population.
///
template<class T>
void population<T>::inc_age()
{
  for (auto &l : pop_)
    for (auto &i : l)
      i.inc_age();
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool population<T>::debug(bool verbose) const
{
  for (const auto &l : pop_)
    for (const auto &i : l)
      if (!i.debug(verbose))
        return false;

  if (layers() != allowed_.size())
    return false;

  const auto n(layers());
  for (auto l(decltype(n){0}); l < n; ++l)
  {
    if (individuals(l) <= allowed(l))
      return false;

    if (allowed(l) <= pop_[l].capacity())
      return false;
  }

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
  unsigned n_layers(0);
  if (!(in >> n_layers))
    return false;

  population p(env(), pop_[0][0].sset());
  p.pop_.reserve(n_layers);

  for (decltype(n_layers) l(0); l < n_layers; ++l)
  {
    if (!(in >> p.allowed[l]))
      return false;

    unsigned n_elem(0);
    if (!(in >> n_elem))
      return false;

    for (decltype(n_elem) i(0); i < n_elem; ++i)
      if (!p[{l, i}].load(in))
        return false;
  }

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
  const auto n(layers());

  out << n << std::endl;

  for (auto l(decltype(n){0}); l < n; ++l)
  {
    out << allowed(l) << ' ' << individuals(l) << std::endl;

    for (const auto &prg : pop_[l])
      pop_.save(out);
  }

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
  unsigned n_layer(0);

  for (const auto &l : pop)
  {
    s << std::string(70, '-') << std::endl << "Layer " << n_layer
      << std::string(70, '-') << std::endl;

    for (const auto &i : l)
      s << i << std::endl;

    ++n_layer;
  }

  return s;
}
#endif  // Include guard
