/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_I_GA_H)
#define      VITA_GA_I_GA_H

#include "kernel/individual.h"

namespace vita
{

///
/// An GA-individual optimized for combinatorial optimization.
///
class i_ga : public individual<i_ga>
{
public:
  i_ga() = default;
  explicit i_ga(const problem &);

  // Iterators.
  using genome_t       = std::vector<int>;
  using const_iterator = genome_t::const_iterator;
  using iterator       = genome_t::iterator;
  using value_type     = genome_t::value_type;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  value_type operator[](std::size_t i) const
  {
    Expects(i < parameters());
    return genome_[i];
  }

  value_type &operator[](std::size_t i)
  {
    Expects(i < parameters());
    signature_.clear();
    return genome_[i];
  }

  operator std::vector<value_type>() const;
  i_ga &operator=(const std::vector<value_type> &);

  // Recombination operators.
  unsigned mutation(double, const problem &);

  ///
  /// \return `true` if the individual is empty, `false` otherwise
  ///
  bool empty() const { return size() == 0; }

  ///
  /// \return the number of parameters stored in the individual
  ///
  /// \note `parameters()` and `size()` are aliases.
  ///
  std::size_t size() const { return genome_.size(); }

  ///
  /// \return the number of parameters stored in the individual
  ///
  /// \note `size()` and `parameters()` are aliases.
  ///
  std::size_t parameters() const { return size(); }

  hash_t signature() const;

  bool operator==(const i_ga &) const;
  unsigned distance(const i_ga &) const;

  // Visualization/output methods.
  void graphviz(std::ostream &) const;

  bool is_valid() const;

  friend class individual<i_ga>;
  friend i_ga crossover(const i_ga &, const i_ga &);

private:
  // *** Private support methods ***
  hash_t hash() const;

  // Serialization.
  bool load_impl(std::istream &, const symbol_set &);
  bool save_impl(std::ostream &) const;

  // *** Private data members ***

  // This is the genome: the entire collection of genes (the entirety of an
  // organism's hereditary information).
  genome_t genome_;
};  // class i_ga

// Recombination operators.
i_ga crossover(const i_ga &, const i_ga &);

// Visualization/output methods.
std::ostream &in_line(const i_ga &, std::ostream & = std::cout);
std::ostream &operator<<(std::ostream &, const i_ga &);

///
/// \return a const iterator pointing to the first individual of the team
///
inline i_ga::const_iterator i_ga::begin() const
{
  return genome_.begin();
}

///
/// \return a const iterator pointing to a end-of-team sentry
///
inline i_ga::const_iterator i_ga::end() const
{
  return genome_.end();
}

///
/// \return an iterator pointing to the first individual of the team
///
inline i_ga::iterator i_ga::begin()
{
  return genome_.begin();
}

///
/// \return an iterator pointing to a end-of-team sentry
///
inline i_ga::iterator i_ga::end()
{
  return genome_.end();
}

}  // namespace vita

#endif  // include guard
