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
/// \param[in] e base vita::environment.
///
/// Creates a random population (initial size \a e.individuals).
///
template<class T>
population<T>::population(const environment &e) : pop_(1)
{
  assert(e.debug(true, true));

  pop_[0].reserve(e.individuals);

  // DO NOT CHANGE with a call to init_layer(0): when layer 0 is empty, env()
  // is undefined!
  for (unsigned i(0); i < e.individuals; ++i)
    pop_[0].emplace_back(e, true);

  assert(debug(true));
}

///
/// \param[in] e an environment for individuals initialization.
/// \param[in] l a layer of the population.
///
/// Resets layer \l of the population.
///
/// \warning
/// If layer 0 is empty then \c env() is undefined! So do not call
/// \c init_layer() when \c individuals(0) == 0 (call \c init_layer(env)).
///
template<class T>
void population<T>::init_layer(const environment *e, unsigned l)
{
  assert(l < pop_.size());
  assert(e || (pop_.size() && pop_[0].size()));

  if (!e)
    e = &env();

  pop_[l].clear();

  for (unsigned i(0); i < e->individuals; ++i)
    pop_[l].emplace_back(*e, true);
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
  assert(pop_.size() <= env().layers);

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
  assert(pop_.size() && pop_[0].size());

  const auto &e(env());

  pop_.insert(pop_.begin(), std::vector<individual>());
  pop_.front().reserve(e.individuals);

  init_layer(&e, 0);
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
  pop_[l].push_back(i);
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
  for (unsigned l(0); l < layers(); ++l)
    n += individuals(l);

  return n;
}

///
/// \return a constant reference to the active environment.
///
template<class T>
const environment &population<T>::env() const
{
  assert(pop_.size());     // DO NOT CHANGE with assert(layers()) => infinite
                           // loop
  assert(pop_[0].size());  // DO NOT CHANGE with assert(individuals(0)) =>
                           // infinite loop

  return pop_[0][0].env();
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
/// \return an iterator pointing one element past the last individual of the
///         population
///
template<class T>
typename population<T>::const_iterator population<T>::end() const
{
  return pop_.end();
}

///
/// \param[in] c the coordinates of an individual.
/// \return \c true if the individual at coordinates \c is too old for his
///         layer.
///
template<class T>
bool population<T>::aged(coord c) const
{
  return pop_[c.layer][c.index].age > max_age(c.layer);
}

///
/// \param[in] l a layer.
/// \return the maximum allowed age for an individual in layer \a l.
///
template<class T>
unsigned population<T>::max_age(unsigned l) const
{
  assert(l < layers());

  if (l + 1 == env().layers)
    return std::numeric_limits<unsigned>::max();

  const auto age_gap(env().alps.age_gap);

  // This is a polynomial aging scheme.
  switch (l)
  {
  case 0:   return age_gap;
  case 1:   return age_gap + age_gap;
  default:  return l * l * age_gap;
  }

  // A linear aging scheme.
  // return age_gap * (l + 1);

  // An exponential aging scheme.
  // switch (l)
  // {
  // case 0:  return age_gap;
  // case 1:  return age_gap + age_gap;
  // default:
  // {
  //   auto k(4);
  //   for (unsigned i(2); i < layer; ++i)
  //     k *= 2;
  //   return k * age_gap;
  // }

  // Fibonacci aging scheme.
  // auto num1(age_gap), num2(age_gap);
  // while (num2 <= 2)
  // {
  //   auto num3(num2);
  //   num2 += num1;
  //   num1 = num3;
  // }
  //
  // if (l == 1)
  //   return num1 + num2 - 1;
  //
  // for (unsigned i(1); i <= l; ++i)
  // {
  //   auto num3(num2);
  //   num2 += num1 -1;
  //   num1 = num3;
  // }
  // return num2;
}

///
/// Increments the age of each individual in the population.
///
template<class T>
void population<T>::inc_age()
{
  for (auto &l : pop_)
    for (auto &i : l)
      ++i.age;
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

  population p(env());
  p.pop_.reserve(n_layers);

  for (unsigned l(0); l < n_layers; ++l)
  {
    unsigned n_elem(0);
    if (!(in >> n_elem))
      return false;

    for (unsigned i(0); i < n_elem; ++i)
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
  out << layers() << std::endl;

  for (const auto &l : pop_)
  {
    out << l.size() << std::endl;

    for (const auto &i : l)
      i.save(out);
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
#endif  // POPULATION_INL_H
