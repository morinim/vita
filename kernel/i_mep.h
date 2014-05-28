/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_I_MEP_H)
#define      VITA_I_MEP_H

#include <cmath>
#include <functional>
#include <iomanip>
#include <set>

#include "kernel/function.h"
#include "kernel/gene.h"
#include "kernel/locus.h"
#include "kernel/matrix.h"
#include "kernel/symbol_set.h"
#include "kernel/ttable.h"
#include "kernel/vitafwd.h"

namespace vita
{
  ///
  /// A single member of a \a population. Each individual contains a genome
  /// which represents a possible solution to the task being tackled (i.e. a
  /// point in the search space).
  ///
  class i_mep
  {
  public:
    i_mep(const environment &, const symbol_set &);

    // Visualization/output methods
    void dump(std::ostream &) const;
    void graphviz(std::ostream &, const std::string & = "") const;
    void in_line(std::ostream &) const;
    void list(std::ostream &) const;
    void tree(std::ostream &) const;

    // Recombination operators
    unsigned mutation()
    { assert(env_->p_mutation >= 0.0); return mutation(env_->p_mutation); }
    unsigned mutation(double);
    i_mep crossover(i_mep) const;

    std::vector<locus> blocks() const;
    i_mep destroy_block(index_t) const;
    i_mep get_block(const locus &) const;

    i_mep replace(const gene &) const;
    i_mep replace(const locus &, const gene &) const;
    i_mep replace(const std::vector<gene> &) const;

    std::pair<i_mep, std::vector<locus>> generalize(unsigned) const;

    bool operator==(const i_mep &) const;
    unsigned distance(const i_mep &) const;

    hash_t signature() const;

    /// This is a measure of how long an individual's family of genotypic
    /// material has been in the population. Randomly generated individuals,
    /// such as those that are created when the search algorithm are started,
    /// start with an age of 0. Each generation that an individual stays in the
    /// population (such as through elitism) its age is increased by one.
    /// Individuals that are created through mutation or recombination take the
    /// age of their oldest parent.
    /// This differs from conventional measures of age, in which individuals
    /// created through applying some type of variation to an existing
    /// individual (e.g. mutation or recombination) start with an age of 0.
    unsigned age() const { return age_; }
    void inc_age() { ++age_; }

    const environment &env() const { return *env_; }
    const symbol_set &sset() const { return *sset_; }

    bool debug(bool = true) const;

    ///
    /// \param[in] l locus of a \c gene.
    /// \return the l-th \c gene of \a this \c individual.
    ///
    const gene &operator[](const locus &l) const { return genome_(l); }

    class const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    ///
    /// \return the total size of the individual (effective size + introns).
    ///
    /// The size is constant for any individual (it's chosen at initialization
    /// time).
    /// \see eff_size()
    ///
    unsigned size() const { return genome_.rows(); }

    unsigned eff_size() const;

    category_t category() const;

    ///
    /// \param[in] l locus of a \c gene.
    /// \param[in] g a new gene.
    ///
    /// Set locus \a l of the genome to \a g. Please note that this is one of
    /// the very few methods that aren't const.
    ///
    void set(const locus &l, const gene &g)
    {
      genome_(l) = g;
      signature_.clear();
    }

    friend class interpreter<i_mep>;

  public:   // Serialization
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support methods
    hash_t hash() const;
    void in_line(std::ostream &, const locus &) const;
    void pack(const locus &, std::vector<unsigned char> *const) const;
    void tree(std::ostream &, const locus &, unsigned, const locus &) const;

  private:  // Private data members
    // This is the genome: the entire collection of genes (the entirety of an
    // organism's hereditary information).
    matrix<gene> genome_;

    // Note that syntactically distinct (but logically equivalent) individuals
    // have the same signature. This is a very interesting  property, useful
    // for individual comparison, information retrieval, entropy calculation...
    mutable hash_t signature_;

    // Starting point of the active code in this individual (the best sequence
    // of genes is starting here).
    locus best_;

    unsigned age_;

    const environment  *env_;
    const symbol_set  *sset_;
  };  // class i_mep

  std::ostream &operator<<(std::ostream &, const i_mep &);

#include "kernel/i_mep_iterator_inl.h"

  ///
  /// \return an iterator to the first active locus of the individual.
  ///
  inline i_mep::const_iterator i_mep::begin() const
  { return i_mep::const_iterator(*this); }

  ///
  /// \return an iterator used as sentry value to stop a cycle.
  ///
  inline i_mep::const_iterator i_mep::end() const
  { return i_mep::const_iterator(); }

  ///
  /// \example example1.cc
  /// Creates a random individual and shows its content.
  ///
  /// \example example3.cc
  /// Performs three types of crossover between two random individuals.
  ///
}  // namespace vita

#endif  // Include guard
