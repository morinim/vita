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

#include "kernel/symbol_set.h"
#include "kernel/ttable.h"

namespace vita
{
  ///
  class i_num_ga
  {
  public:
    explicit i_num_ga(const environment &, const symbol_set &);

    // Visualization/output methods
    void graphviz(std::ostream &) const;
    std::ostream &in_line(std::ostream &) const;
    std::ostream &list(std::ostream &) const;
    std::ostream &tree(std::ostream &) const;

    // Recombination operators
    unsigned mutation();
    { assert(env_->p_mutation >= 0.0); return mutation(env_->p_mutation); }
    unsigned mutation(double);
    i_num_ga crossover(i_num_ga) const;

    using const_iterator = typename std::vector<gene>::const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    ///
    /// \param[in] l locus of a \c gene.
    /// \return the l-th \c gene of \a this \c individual.
    ///
    const gene &operator[](unsigned i) const
    { assert(i < size(); return genome_[i]; }

    ///
    /// \return the size of the individual.
    ///
    /// The size is constant for any individual (it's chosen at initialization
    /// time).
    ///
    unsigned size() const { return genome_.size(); }

    hash_t signature() const;

    bool operator==(const team<T> &) const;
    unsigned distance(const team<T> &) const;

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

  private:  // Private support methods
    hash_t hash() const;
    void pack(std::vector<unsigned char> *const) const;

  private:  // Private data members
    // This is the genome: the entire collection of genes (the entirety of an
    // organism's hereditary information).
    std::vector<gene> genome_;

    mutable hash_t signature_;

    unsigned age_;

    const environment  *env_;
    const symbol_set  *sset_;
  };  // class i_num_ga
}  // namespace vita

#endif  // Include guard
