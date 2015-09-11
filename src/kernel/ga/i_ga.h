/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014, 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_I_GA_H)
#define      VITA_GA_I_GA_H

#include "kernel/gene.h"
#include "kernel/individual.h"

namespace vita
{
///
/// \brief An individual optimized for genetic algorithms
///
/// This is a special case of a GP individual. Logically it seems a GP
/// individual with as many categories as parameters to be optimized and
/// index dimension equal to 1 (of course internal representation is
/// different).
///
/// This way, having the same interface of i_mep, this class is compatible
/// with many GP algorithms (and is directly suitable for combinatorial
/// optimization).
///
/// The class also adds a special three term crossover operator and other
/// methods to support differential evolution.
///
/// \see
/// "Differential evolution" - Kenneth Price, Rainer Storn (DDJ #264 april
/// 1997).
///
class i_ga : public individual
{
public:
  i_ga() : individual(), genome_() {}

  explicit i_ga(const environment &);

  // Visualization/output methods
  void graphviz(std::ostream &) const;
  virtual std::ostream &in_line(std::ostream &) const override;
  std::ostream &list(std::ostream &) const;
  std::ostream &tree(std::ostream &) const;

  // Recombination operators
  unsigned mutation(double, const symbol_set &);
  i_ga crossover(i_ga) const;
  i_ga crossover(double, const double [2], const i_ga &, const i_ga &,
                 i_ga) const;

  class const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

  operator std::vector<double>() const;

  const gene &operator[](const locus &l) const
  {
    assert(l.index == 0);
    assert(l.category < parameters());
    return genome_[l.category];
  }

  double operator[](unsigned i) const
  { assert(i < parameters()); return genome_[i].par; }

  double &operator[](unsigned i)
  {
    assert(i < parameters());
    signature_.clear();
    return genome_[i].par;
  }

  i_ga &operator=(const std::vector<gene::param_type> &);

  ///
  /// \return `true` if the individual is empty, `0` otherwise.
  ///
  bool empty() const { return !parameters(); }

  ///
  /// \return `1` if the individual is not empty, `0` otherwise.
  ///
  /// This is for compatibility for GP algorithm, but isn't significative
  /// for differential evolution.
  ///
  /// \see i_ga::parameters()
  ///
  unsigned size() const { return !empty(); }
  unsigned categories() const { return parameters(); }

  ///
  /// \return the number of parameters stored in the individual.
  ///
  unsigned parameters() const
  { return static_cast<unsigned>(genome_.size()); }

  hash_t signature() const;

  bool operator==(const i_ga &) const;
  unsigned distance(const i_ga &) const;

  bool debug() const;

private:  // Private support methods
  hash_t hash() const;
  void pack(std::vector<unsigned char> *const) const;

  // NVI implementation (serialization)
  virtual bool load_nvi(std::istream &, const environment &) override;
  virtual bool save_nvi(std::ostream &) const override;

private:  // Private data members
  // This is the genome: the entire collection of genes (the entirety of an
  // organism's hereditary information).
  std::vector<gene> genome_;
};  // class i_ga

std::ostream &operator<<(std::ostream &, const i_ga &);

#include "kernel/ga/i_ga_iterator.tcc"

///
/// \return an iterator pointing to the first individual of the team.
///
inline i_ga::const_iterator i_ga::begin() const
{
  return i_ga::const_iterator(*this);
}

///
/// \return an iterator pointing to a end-of-team sentry.
///
inline i_ga::const_iterator i_ga::end() const
{
  return i_ga::const_iterator();
}

}  // namespace vita

#endif  // Include guard
