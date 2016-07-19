/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_I_DE_H)
#define      VITA_GA_I_DE_H

#include "kernel/gene.h"
#include "kernel/individual.h"

namespace vita
{
///
/// \brief An individual optimized for differential evolution.
///
/// \see
/// "Differential evolution" - Kenneth Price, Rainer Storn (DDJ #264 april
/// 1997).
///
class i_de : public individual<i_de>
{
public:
  i_de() = default;
  explicit i_de(const environment &);

  // Visualization/output methods
  void graphviz(std::ostream &) const;
  std::ostream &in_line(std::ostream &) const;

  // Recombination operators
  unsigned mutation(double, const environment &);
  i_de crossover(double, const double [2],
                 const i_de &, const i_de &, const i_de &) const;

  // Iterators.
  using const_iterator = std::vector<gene>::const_iterator;
  using iterator = std::vector<gene>::iterator;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  operator std::vector<double>() const;

  const gene &operator[](const locus &l) const
  {
    Expects(l.index == 0);
    Expects(l.category < parameters());
    return genome_[l.category];
  }

  double operator[](unsigned i) const
  {
    Expects(i < parameters());
    return genome_[i].par;
  }

  double &operator[](unsigned i)
  {
    Expects(i < parameters());
    signature_.clear();
    return genome_[i].par;
  }

  i_de &operator=(const std::vector<double> &);

  ///
  /// \return `true` if the individual is empty, `0` otherwise.
  ///
  bool empty() const { return !parameters(); }

  ///
  /// \return the number of parameters stored in the individual.
  ///
  unsigned parameters() const
  { return static_cast<unsigned>(genome_.size()); }

  hash_t signature() const;

  bool operator==(const i_de &) const;

  bool debug() const;

  friend class individual<i_de>;
  friend i_de crossover(const i_de &, const i_de &);

private:
  // *** Private support methods ***
  hash_t hash() const;
  void pack(std::vector<unsigned char> *const) const;

  // Serialization.
  bool load_impl(std::istream &, const environment &);
  bool save_impl(std::ostream &) const;

  // *** Private data members ***

  // This is the genome: the entire collection of genes (the entirety of an
  // organism's hereditary information).
  std::vector<gene> genome_;
};  // class i_de

double distance(const i_de &, const i_de &);

// Recombination operators.
i_de crossover(const i_de &, const i_de &);

std::ostream &operator<<(std::ostream &, const i_de &);

///
/// \return a const iterator pointing to the first individual of the team.
///
inline i_de::const_iterator i_de::begin() const
{
  return genome_.begin();
}

///
/// \return a const iterator pointing to a end-of-team sentry.
///
inline i_de::const_iterator i_de::end() const
{
  return genome_.end();
}

///
/// \return an iterator pointing to the first individual of the team.
///
inline i_de::iterator i_de::begin()
{
  return genome_.begin();
}

///
/// \return an iterator pointing to a end-of-team sentry.
///
inline i_de::iterator i_de::end()
{
  return genome_.end();
}

}  // namespace vita

#endif  // include guard
