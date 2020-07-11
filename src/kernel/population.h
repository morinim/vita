/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
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
#include "kernel/problem.h"
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
  explicit population(const problem &);

  struct coord;
  using layer_t = std::vector<T>;
  T &operator[](coord);
  const T &operator[](coord) const;

  unsigned individuals() const;
  unsigned individuals(unsigned) const;
  unsigned allowed(unsigned) const;

  void init_layer(unsigned);
  void add_layer();
  unsigned layers() const;
  void add_to_layer(unsigned, const T &);
  void pop_from_layer(unsigned);
  void remove_layer(unsigned);
  void set_allowed(unsigned, unsigned);

  void inc_age();

  const problem &get_problem() const;

  bool is_valid() const;

  // Iterators.
  template<bool> class base_iterator;
  using const_iterator = base_iterator<true>;
  using iterator = base_iterator<false>;

  const_iterator begin() const;
  const_iterator end() const;

  // Serialization.
  bool load(std::istream &, const problem &);
  bool save(std::ostream &) const;

private:
  const environment &get_helper(environment *) const;
  const problem &get_helper(problem *) const;

  const problem *prob_;

  std::vector<layer_t> pop_;
  std::vector<unsigned> allowed_;
};

template<class T> typename population<T>::coord pickup(const population<T> &);
template<class T> typename population<T>::coord pickup(
  const population<T> &, typename population<T>::coord);

#include "kernel/population_coord.tcc"
#include "kernel/population_iterator.tcc"
#include "kernel/population.tcc"

}  // namespace vita

#endif  // include guard
