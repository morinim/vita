/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_I_MEP_H)
#define      VITA_I_MEP_H

#include <cmath>
#include <iomanip>

#include "kernel/gp/function.h"
#include "kernel/gp/gene.h"
#include "kernel/individual.h"
#include "utility/matrix.h"

namespace vita
{
///
/// A MEP (Multi Expression Programming) single member of a `population`.
///
/// Each individual contains a genome which represents a possible solution to
/// the task being tackled (i.e. a point in the search space).
///
class i_mep : public individual<i_mep>
{
public:
  i_mep() : individual(), genome_(), best_(locus::npos()),
            active_crossover_type_() {}

  explicit i_mep(const problem &);
  explicit i_mep(const std::vector<gene> &);

  // ---- Recombination operators ----
  enum crossover_t {one_point, two_points, tree, uniform, NUM_CROSSOVERS};

  friend i_mep crossover(const i_mep &, const i_mep &);
  unsigned mutation(double, const problem &);

  // ---- Working with blocks / genome ----
  std::set<locus> blocks() const;
  i_mep destroy_block(index_t, const symbol_set &) const;
  i_mep get_block(const locus &) const;

  i_mep replace(const gene &) const;
  i_mep replace(const locus &, const gene &) const;

  i_mep cse() const;

  bool operator==(const i_mep &) const;

  hash_t signature() const;

  const gene &operator[](locus) const;

  unsigned active_symbols() const;
  category_t categories() const;
  bool empty() const;
  unsigned size() const;

  category_t category() const;
  locus best() const;

  bool is_valid() const;

  // ---- Iterators ----
  template<bool> class basic_iterator;
  using const_iterator = basic_iterator<true>;
  using iterator = basic_iterator<false>;
  using value_type = gene;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  template<bool> friend class basic_iterator;
  friend class individual<i_mep>;
  friend class interpreter<i_mep>;

private:
  // ---- Private support methods ----
  hash_t hash() const;
  void pack(const locus &, std::vector<std::byte> *) const;

  // Serialization.
  bool load_impl(std::istream &, const symbol_set &);
  bool save_impl(std::ostream &) const;

  // ---- Private data members ----

  // This is the genome: the entire collection of genes (the entirety of an
  // organism's hereditary information).
  matrix<gene> genome_;

  // Starting point of the active code in this individual (the best sequence
  // of genes starts here).
  locus best_;

  // Crossover operator used to create this individual. Initially this is set
  // to a random type.
  crossover_t active_crossover_type_;
};  // class i_mep

[[nodiscard]] unsigned distance(const i_mep &, const i_mep &);
[[nodiscard]] locus random_locus(const i_mep &);

///
/// \return the starting locus of the best sequence available
///
inline locus i_mep::best() const
{
  return best_;
}

///
/// \param[in] l locus of a `gene`
/// \return      the `l`-th gene of `this` individual
///
inline const gene &i_mep::operator[](locus l) const
{
  return genome_(l);
}

///
/// \return the total number of categories the individual is using
///
inline category_t i_mep::categories() const
{
  return static_cast<category_t>(genome_.cols());
}

///
/// \return the total size of the individual (effective size + introns)
///
/// \remark Size is constant for any individual (it's chosen at initialization
///         time).
///
/// \see eff_size()
///
inline unsigned i_mep::size() const
{
  return static_cast<unsigned>(genome_.rows());
}

///
/// \return `true` if the individual isn't initialized
///
inline bool i_mep::empty() const
{
  return size() == 0;
}

bool operator!=(const i_mep &, const i_mep &);

std::ostream &operator<<(std::ostream &, const i_mep &);

#include "kernel/gp/mep/i_mep_iterator.tcc"

///
/// \return an iterator to the first active locus of the individual
///
inline i_mep::const_iterator i_mep::begin() const
{
  return i_mep::const_iterator(*this);
}

///
/// \return an iterator used as sentry value to stop a cycle
///
inline i_mep::const_iterator i_mep::end() const
{
  return i_mep::const_iterator();
}

///
/// \return an iterator to the first active locus of the individual
///
inline i_mep::iterator i_mep::begin()
{
  return i_mep::iterator(*this);
}

///
/// \return an iterator used as sentry value to stop a cycle
///
inline i_mep::iterator i_mep::end()
{
  return i_mep::iterator();
}

template<> struct has_introns<i_mep> : std::true_type {};

}  // namespace vita

#endif  // include guard
