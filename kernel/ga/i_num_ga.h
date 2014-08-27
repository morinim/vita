/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_I_NUM_GA_H)
#define      VITA_GA_I_NUM_GA_H

#include "kernel/gene.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \brief An individual optimized for differential evolution method
  ///
  /// This is essentially a real value vector.
  ///
  /// While many genetic algorithms use integers to approximate continuous
  /// parameters, this choice limits the resolution with which an optimum can
  /// be located. Floating point not only uses computer resources efficently,
  /// it also makes input and output transparent for the user. Parameters can
  /// be input, manipulated and output as ordinary floating-point numbers
  /// without ever being reformatted as genes with a different binary
  /// representation.
  ///
  /// The class is compatible with many GP algorithms (having the same
  /// interface of \a i_mep class).
  ///
  /// \a i_num_ga adds the special three terms
  /// crossover operator which is the crucial idea behind DE.
  ///
  /// \see
  /// "Differential evolution" - Kenneth Price, Rainer Storn (DDJ #264).
  ///
  class i_num_ga : public individual
  {
  public:
    explicit i_num_ga(const environment &, const symbol_set &);

    // Visualization/output methods
    void graphviz(std::ostream &) const;
    virtual std::ostream &in_line(std::ostream &) const override;
    std::ostream &list(std::ostream &) const;
    std::ostream &tree(std::ostream &) const;

    // Recombination operators
    unsigned mutation()
    { assert(env_->p_mutation >= 0.0); return mutation(env_->p_mutation); }
    unsigned mutation(double);
    i_num_ga crossover(i_num_ga) const;
    i_num_ga crossover(const i_num_ga &, const i_num_ga &) const;

    //using const_iterator = typename std::vector<gene>::const_iterator;
    class const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

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

    i_num_ga &operator=(const std::vector<gene::param_type> &);

    ///
    /// \return 0.
    ///
    /// This is for compatibility for GP algorithm, but isn't significative
    /// for differential evolution.
    ///
    /// \see i_num_ga::parameters()
    ///
    unsigned size() const { return 0; }

    ///
    /// \return the number of parameters stored in the individual.
    ///
    unsigned parameters() const
    { return static_cast<unsigned>(genome_.size()); }

    hash_t signature() const;

    bool operator==(const i_num_ga &) const;
    unsigned distance(const i_num_ga &) const;

    bool debug(bool = true) const;

  public:   // Serialization
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support methods
    hash_t hash() const;
    void pack(std::vector<unsigned char> *const) const;

  private:  // Private data members
    // This is the genome: the entire collection of genes (the entirety of an
    // organism's hereditary information).
    std::vector<gene> genome_;
  };  // class i_num_ga

  std::ostream &operator<<(std::ostream &, const i_num_ga &);

#include "kernel/ga/i_num_ga_iterator_inl.h"

  ///
  /// \return an iterator pointing to the first individual of the team.
  ///
  inline i_num_ga::const_iterator i_num_ga::begin() const
  {
    return i_num_ga::const_iterator(*this);
  }

  ///
  /// \return an iterator pointing to a end-of-team sentry.
  ///
  inline i_num_ga::const_iterator i_num_ga::end() const
  {
    return i_num_ga::const_iterator();
  }
}  // namespace vita

#endif  // Include guard
