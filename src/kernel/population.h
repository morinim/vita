/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_POPULATION_H)
#define      VITA_POPULATION_H

#include <fstream>

#include "kernel/environment.h"
#include "kernel/log.h"
#include "kernel/random.h"

namespace vita
{
///
/// A group of individuals which may interact together (for example by mating)
/// producing offspring.
///
/// \tparam T the type of the an individual
///
/// Typical population size in GP ranges from ten to many thousands. The
/// population is organized in one or more layers that can interact in
/// many ways (depending on the evolution strategy).
///
template<class T>
class population
{
public:
  struct coord;
  using layer_t = std::vector<T>;

  explicit population(const environment &);

  T &operator[](coord);
  const T &operator[](coord) const;

  unsigned allowed(unsigned) const;
  unsigned individuals() const;
  unsigned individuals(unsigned) const;

  void init_layer(unsigned);
  void add_layer();
  unsigned layers() const;
  void inc_age();
  void add_to_layer(unsigned, const T &);
  void pop_from_layer(unsigned);
  void set_allowed(unsigned, unsigned);

  const environment &env() const;

  bool debug() const;

  // Iterators.
  template<bool> class base_iterator;
  using const_iterator = base_iterator<true>;
  using iterator = base_iterator<false>;

  const_iterator begin() const;
  const_iterator end() const;

  // Serialization.
  bool load(std::istream &, const environment &);
  bool save(std::ostream &) const;

private:
  const environment *env_;

  std::vector<layer_t> pop_;
  std::vector<unsigned> allowed_;
};

template<class T> typename population<T>::coord pickup();
template<class T> typename population<T>::coord pickup(
  const population<T> &, typename population<T>::coord);


#include "kernel/population_coord.tcc"
#include "kernel/population_iterator.tcc"
#include "kernel/population.tcc"

}  // namespace vita

#endif  // include guard
