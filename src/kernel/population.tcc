/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_POPULATION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_POPULATION_TCC)
#define      VITA_POPULATION_TCC

///
/// \param[in] e base vita::environment.
///
/// Creates a random population (initial size `e.individuals`).
///
template<class T>
population<T>::population(const environment &e) : env_(&e), pop_(1),
                                                  allowed_(1)
{
  assert(e.debug(true, true));
  assert(e.sset);

  const auto n(e.individuals);
  pop_[0].reserve(n);
  allowed_[0] = n;

  init_layer(0);

  assert(debug(true));
}

///
/// \param[in] l a layer of the population.
///
/// Resets layer `l` of the population.
///
/// \warning
/// If layer `l` is nonexistent/empty the method doesn't work!
///
template<class T>
void population<T>::init_layer(unsigned l)
{
  assert(l < layers());

  pop_[l].clear();

  const auto n(allowed(l));
  for (auto i(decltype(n){0}); i < n; ++i)
    pop_[l].emplace_back(env());
}

///
/// \return number of active layers.
///
/// \note
/// * The number of active layers is a dynamic value (it is a monotonically
///   increasing function of the generation number).
/// * Maximum number of layers (`env.alps.layers`) is a constant value
///   greater than or equal to `layers()`.
///
template<class T>
unsigned population<T>::layers() const
{
  return static_cast<unsigned>(pop_.size());
}

///
/// \brief Add a new layer to the population
///
/// The new layer is inserted as the lower layer and randomly initialized.
///
template<class T>
void population<T>::add_layer()
{
  assert(layers());
  assert(individuals(0));

  pop_.insert(pop_.begin(), layer_t());
  pop_[0].reserve(env_->individuals);

  allowed_.insert(allowed_.begin(), env_->individuals);

  init_layer(0);
}

///
/// \param[in] l index of a layer.
/// \param[in] i an individual.
///
/// Add individual `i` to layer `l`.
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
/// Remove the last individual of layer `l`.
///
template<class T>
void population<T>::pop_from_layer(unsigned l)
{
  assert(l < layers());
  pop_[l].pop_back();
}


///
/// \param[in] c coordinates of an individual.
/// \return a reference to the individual at coordinates `c`.
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
/// \return a constant reference to the individual at coordinates `c`.
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
/// \return the number of individuals allowed in layer `l`.
///
/// \note `for each l: individuals(l) < allowed(l)`
///
template<class T>
unsigned population<T>::allowed(unsigned l) const
{
  assert(l < layers());
  return allowed_[l];
}

///
/// \param[in] l a layer.
/// \param[in] n number of programs allowed in layer `l`.
///
/// Sets the number of programs allowed in layer `l`. If layer `l` contains
/// more programs than the allowed, the surplus will be deleted.
///
template<class T>
void population<T>::set_allowed(unsigned l, unsigned n)
{
  assert(l < layers());
  assert(n <= pop_[l].capacity());

  if (individuals(l) > n)
  {
    const auto delta(individuals(l) - n);

    // We should consider the remove-erase idiom for deleting delta random
    // elements.
    if (delta)
      pop_[l].erase(pop_[l].end() - delta, pop_[l].end());
  }

  allowed_[l] = n;

  assert(debug(true));
}

///
/// \param[in] l a layer.
/// \return the number of individuals in layer `l`.
///
template<class T>
unsigned population<T>::individuals(unsigned l) const
{
  assert(l < layers());
  return static_cast<unsigned>(pop_[l].size());
}

///
/// \return the number of individuals in the population.
///
template<class T>
unsigned population<T>::individuals() const
{
  unsigned n(0);
  for (const auto &layer : pop_)
    n += static_cast<unsigned>(layer.size());

  return n;
}

///
/// \return a constant reference to the active environment.
///
template<class T>
const environment &population<T>::env() const
{
  assert(env_);
  return *env_;
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
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the object passes the internal consistency check.
///
template<class T>
bool population<T>::debug(bool verbose) const
{
  for (const auto &l : pop_)
    for (const auto &i : l)
      if (!i.debug(verbose))
        return false;

  if (layers() != allowed_.size())
  {
    if (verbose)
      std::cerr << k_s_debug
                << "Number of layers doesn't match allowed array size.\n";
    return false;
  }

  const auto n(layers());
  for (auto l(decltype(n){0}); l < n; ++l)
  {
    if (allowed(l) < individuals(l))
      return false;

    if (pop_[l].capacity() < allowed(l))
      return false;
  }

  if (!env_)
  {
    if (verbose)
      std::cerr << k_s_debug << "Undefined environment.\n";
    return false;
  }

  return true;
}

///
/// \param[in] e environment used to build the individual.
/// \param[in] in input stream.
/// \return `true` if population was loaded correctly.
///
/// \note
/// If the load operation isn't successful the current population isn't
/// changed.
///
template<class T>
bool population<T>::load(std::istream &in, const environment &e)
{
  unsigned n_layers;
  if (!(in >> n_layers) || !n_layers)
    return false;

  population p(e);
  p.pop_.reserve(n_layers);
  p.allowed_.reserve(n_layers);

  for (decltype(n_layers) l(0); l < n_layers; ++l)
  {
    if (!(in >> p.allowed_[l]))
      return false;

    unsigned n_elem(0);
    if (!(in >> n_elem))
      return false;

    for (decltype(n_elem) i(0); i < n_elem; ++i)
      if (!p[{l, i}].load(in, e))
        return false;
  }

  *this = p;
  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if population was saved correctly.
///
template<class T>
bool population<T>::save(std::ostream &out) const
{
  const auto n(layers());

  out << n << '\n';

  for (auto l(decltype(n){0}); l < n; ++l)
  {
    out << allowed(l) << ' ' << individuals(l) << '\n';

    for (const auto &prg : pop_[l])
      prg.save(out);
  }

  return out.good();
}

///
/// \param[in] p the population (we extract the coordinates of an individual
///              of `p`).
/// \param[in] target coordinates of a reference individual.
/// \return the coordinates of a random individual "near" `target`.
///
/// Other parameters from the environment:
/// * mate_zone - to restrict the selection of individuals to a segment of
///   the population.
///
template<class T>
typename population<T>::coord pickup(const population<T> &p,
                                     typename population<T>::coord target)
{
  return {target.layer, random::ring(target.index, *p.env().mate_zone,
                                     p.individuals(target.layer))};
}

///
/// \param[in,out] s output stream.
/// \param[in] pop population to be listed.
/// \return the output stream.
///
template<class T>
std::ostream &operator<<(std::ostream &s, const population<T> &pop)
{
  unsigned n_layer(0);

  for (const auto &l : pop)
  {
    s << std::string(70, '-') << "\nLayer " << n_layer
      << std::string(70, '-') << '\n';

    for (const auto &i : l)
      s << i << '\n';

    ++n_layer;
  }

  return s;
}
#endif  // Include guard
