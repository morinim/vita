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

#if !defined(VITA_POPULATION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_POPULATION_TCC)
#define      VITA_POPULATION_TCC

///
/// Creates a random population.
///
/// \param[in] e current problem
///
/// Starting size of the population is `e.individuals`.
///
template<class T>
population<T>::population(const problem &p) : prob_(&p), pop_(1),
                                              allowed_(1)
{
  const auto n(p.env.individuals);
  pop_[0].reserve(n);
  allowed_[0] = n;

  init_layer(0);

  Ensures(is_valid());
}

///
/// Resets layer `l` of the population.
///
/// \param[in] l a layer of the population
///
/// \warning If layer `l` is nonexistent/empty the method doesn't work!
///
template<class T>
void population<T>::init_layer(unsigned l)
{
  Expects(l < layers());

  pop_[l].clear();

  std::generate_n(std::back_inserter(pop_[l]), allowed(l),
                  [this] {return T(get_problem()); });
}

///
/// \return number of active layers
///
/// \note
/// * The number of active layers is a dynamic value (almost monotonically
///   increasing with the generation number).
/// * Maximum number of layers (`env.alps.layers`) is a constant value
///   greater than or equal to `layers()`.
///
template<class T>
unsigned population<T>::layers() const
{
  return static_cast<unsigned>(pop_.size());
}

///
/// Adds a new layer to the population.
///
/// The new layer is inserted as the lower layer and randomly initialized.
///
template<class T>
void population<T>::add_layer()
{
  Expects(layers());
  Expects(individuals(0));

  const auto individuals(get_problem().env.individuals);

  pop_.insert(pop_.begin(), layer_t());
  pop_[0].reserve(individuals);

  allowed_.insert(allowed_.begin(), individuals);

  init_layer(0);
}

template<class T>
void population<T>::remove_layer(unsigned l)
{
  Expects(l);
  Expects(l < layers());

  pop_.erase(std::next(pop_.begin(), l));
  allowed_.erase(std::next(allowed_.begin(), l));
}

///
/// Adds individual `i` to layer `l`.
///
/// \param[in] l index of a layer
/// \param[in] i an individual
///
template<class T>
void population<T>::add_to_layer(unsigned l, const T &i)
{
  Expects(l < layers());

  if (individuals(l) < allowed(l))
    pop_[l].push_back(i);
}

///
/// Removes the last individual of layer `l`.
///
/// \param[in] l index of a layer
///
template<class T>
void population<T>::pop_from_layer(unsigned l)
{
  Expects(l < layers());
  pop_[l].pop_back();
}


///
/// \param[in] c coordinates of an individual
/// \return      a reference to the individual at coordinates `c`
///
template<class T>
T &population<T>::operator[](coord c)
{
  Expects(c.layer < layers());
  Expects(c.index < individuals(c.layer));
  return pop_[c.layer][c.index];
}

///
/// \param[in] c coordinates of an individual
/// \return      a constant reference to the individual at coordinates `c`
///
template<class T>
const T &population<T>::operator[](coord c) const
{
  Expects(c.layer < layers());
  Expects(c.index < individuals(c.layer));
  return pop_[c.layer][c.index];
}

///
/// \param[in] l a layer
/// \return      the number of individuals allowed in layer `l`
///
/// \note `for each l individuals(l) < allowed(l)`
///
template<class T>
unsigned population<T>::allowed(unsigned l) const
{
  Expects(l < layers());
  return allowed_[l];
}

///
/// Sets the number of programs allowed in layer `l`.
///
/// \param[in] l a layer
/// \param[in] n number of programs allowed in layer `l`
///
/// If layer `l` contains more programs than the amount allowed, the surplus
/// is deleted.
///
template<class T>
void population<T>::set_allowed(unsigned l, unsigned n)
{
  Expects(l < layers());
  Expects(n <= pop_[l].capacity());

  const auto &env(get_problem().env);
  // Unless explicitly allowed by the environment, do not drop under a
  // predefined number of individuals.
  const auto min(std::min(env.min_individuals, env.individuals));
  n = std::max(n, min);

  if (individuals(l) > n)
  {
    const auto delta(individuals(l) - n);

    // We should consider the remove-erase idiom for deleting delta random
    // elements.
    if (delta)
      pop_[l].erase(pop_[l].end() - delta, pop_[l].end());
  }

  allowed_[l] = n;

  Ensures(is_valid());
}

///
/// \param[in] l a layer
/// \return      the number of individuals in layer `l`
///
template<class T>
unsigned population<T>::individuals(unsigned l) const
{
  Expects(l < layers());
  return static_cast<unsigned>(pop_[l].size());
}

///
/// \return the number of individuals in the population
///
template<class T>
unsigned population<T>::individuals() const
{
  using ret_t = decltype(individuals());

  return std::accumulate(pop_.begin(), pop_.end(), ret_t(0),
                         [](auto accumulator, const auto &layer)
                         {
                           return accumulator
                                  + static_cast<ret_t>(layer.size());
                         });
}

///
/// \return a constant reference to the active problem
///
template<class T>
const problem &population<T>::get_problem() const
{
  Expects(prob_);
  return *prob_;
}

///
/// \return a `const_iterator` pointing to the first layer of the population
///
/// \warning Pointer to the first LAYER *NOT* to the first PROGRAM.
///
template<class T>
typename population<T>::const_iterator population<T>::begin() const
{
  return const_iterator(*this, true);
}

///
/// \return an iterator pointing one element past the last individual of the
///         population
///
template<class T>
typename population<T>::const_iterator population<T>::end() const
{
  return const_iterator(*this, false);
}

///
/// Increments the age of each individual of the population
///
template<class T>
void population<T>::inc_age()
{
  for (auto &l : pop_)
    for (auto &i : l)
      i.inc_age();
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T>
bool population<T>::is_valid() const
{
  for (const auto &l : pop_)
    for (const auto &i : l)
      if (!i.is_valid())
        return false;

  if (layers() != allowed_.size())
  {
    vitaERROR << "Number of layers doesn't match allowed array size";
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

  if (!prob_)
  {
    vitaERROR << "Undefined `problem`";
    return false;
  }

  return true;
}

///
/// \param[in] prob current problem
/// \param[in] in   input stream
/// \return         `true` if population was loaded correctly
///
/// \note The current population isn't changed if the load operation fails.
///
template<class T>
bool population<T>::load(std::istream &in, const problem &prob)
{
  unsigned n_layers;
  if (!(in >> n_layers) || !n_layers)
    return false;

  population p(prob);
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
      if (!p[{l, i}].load(in, prob.sset))
        return false;
  }

  *this = p;
  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if population was saved correctly
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
/// \param[in] p a population
/// \return      the index of a random individual in `p`
///
/// \related population
///
template<class T>
typename population<T>::coord pickup(const population<T> &p)
{
  const auto n_layers(p.layers());

  if (n_layers == 1)
    return {0, random::sup(p.individuals(0))};

  // With multiple layers we cannot be sure that every layer has the same
  // number of individuals. So the simple (and fast) solution:
  //
  //     const auto l(random::sup(n_layers));
  //     return return {l, random::sup(p.individuals(l)};
  //
  // isn't appropriate.

  std::vector<unsigned> s(n_layers);
  std::generate(s.begin(), s.end(),
                [&p, l = 0] () mutable { return p.individuals(l++); });

  std::discrete_distribution<unsigned> dd(s.begin(), s.end());
  const auto l(dd(random::engine));
  return {l, random::sup(s[l])};
}

///
/// \param[in] p      a population
/// \param[in] target coordinates of a reference individual
/// \return           the coordinates of a random individual *near* `target`
///
/// \related population
///
/// Other parameters from the environment:
/// * mate_zone - restricts the selection of individuals to a segment of the
///   population.
///
template<class T>
typename population<T>::coord pickup(const population<T> &p,
                                     typename population<T>::coord target)
{
  const auto mate_zone(p.get_problem().env.mate_zone);
  const auto individuals(p.individuals(target.layer));

  return {target.layer, random::ring(target.index, mate_zone, individuals)};
}

///
/// \param[in,out] s   output stream
/// \param[in]     pop population to be listed
/// \return            the output stream
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
#endif  // include guard
