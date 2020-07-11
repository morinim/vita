/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_I_DE_H)
#define      VITA_GA_I_DE_H

#include "kernel/individual.h"
#include "kernel/range.h"

namespace vita
{
///
/// An individual optimized for differential evolution.
///
/// \see https://github.com/morinim/vita/wiki/bibliography#10
///
class i_de : public individual<i_de>
{
public:
  i_de() = default;
  explicit i_de(const problem &);

  // Iterators.
  using genome_t       = std::vector<double>;
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
  i_de &operator=(const std::vector<value_type> &);

  i_de crossover(double, const range_t<double> &,
                 const i_de &, const i_de &, const i_de &) const;

  /// \return `true` if the individual is empty, `false` otherwise
  bool empty() const { return !parameters(); }

  /// \return the number of parameters stored in the individual
  std::size_t parameters() const { return genome_.size(); }

  hash_t signature() const;

  // Visualization/output methods.
  void graphviz(std::ostream &) const;

  bool is_valid() const;

  friend class individual<i_de>;

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
};  // class i_de

bool operator==(const i_de &, const i_de &);
double distance(const i_de &, const i_de &);

// Visualization/output functions.
std::ostream &in_line(const i_de &, std::ostream & = std::cout);
std::ostream &operator<<(std::ostream &, const i_de &);

///
/// \return a const iterator pointing to the first gene
///
inline i_de::const_iterator i_de::begin() const
{
  return genome_.begin();
}

///
/// \return a const iterator pointing to (one) past the last gene
///
inline i_de::const_iterator i_de::end() const
{
  return genome_.end();
}

///
/// \return an iterator pointing to the first gene
///
inline i_de::iterator i_de::begin()
{
  return genome_.begin();
}

///
/// \return an iterator pointing to (one) past the last gene
///
inline i_de::iterator i_de::end()
{
  return genome_.end();
}

}  // namespace vita

#endif  // include guard
