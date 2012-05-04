/**
 *
 *  \file individual.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(INDIVIDUAL_H)
#define      INDIVIDUAL_H

#include <cmath>
#include <functional>
#include <iomanip>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "kernel/environment.h"
#include "kernel/gene.h"
#include "kernel/locus.h"
#include "kernel/ttable.h"

namespace vita
{
  ///
  /// A single member of a \a population. Each individual contains a genome
  /// which represents a possible solution to the task being tackled (i.e. a
  /// point in the search space).
  ///
  class individual
  {
  public:
    typedef std::function<individual (const individual &, const individual &)>
    crossover_wrapper;

    individual(const environment &, bool);

    void dump(std::ostream &) const;
    void graphviz(std::ostream &, const std::string & = "") const;
    void in_line(std::ostream &) const;
    void list(std::ostream &) const;
    void tree(std::ostream &) const;

    individual crossover(const individual &) const;
    unsigned mutation() { return mutation(*env_->p_mutation); }
    unsigned mutation(double);

    std::list<locus> blocks() const;
    individual destroy_block(unsigned) const;
    individual get_block(const locus &) const;
    individual replace(const symbol_ptr &, const std::vector<unsigned> &) const;
    individual replace(const symbol_ptr &, const std::vector<unsigned> &,
                       const locus &) const;

    individual generalize(unsigned, std::vector<locus> *const) const;

    bool operator==(const individual &) const;
    bool operator!=(const individual &x) const { return !(*this == x); }
    unsigned distance(const individual &) const;

    hash_t signature() const;

    const environment &env() const { return *env_; }

    bool check(bool = true) const;

    ///
    /// \param[in] l locus of a \c gene.
    /// \return the l-th \c gene of \a this \c individual.
    ///
    const gene &operator[](const locus &l) const { return genome_(l); }

    ///
    /// \return the total size of the individual (effective size + introns).
    ///
    /// The size is constant for any individual (it's choosen at initialization
    /// time).
    /// \see eff_size
    ///
    unsigned size() const { return genome_.size(); }

    unsigned eff_size() const;

    category_t category() const;

    ///
    /// \param[in] locus locust of a \c gene.
    /// \param[in] g a new gene.
    ///
    /// Set the \a locus of the genome to \a g. Please note that this is one of
    /// the very few methods that aren't const.
    ///
    void set(const locus &locus, const gene &g)
    {
      genome_(locus) = g;
      signature_.clear();
    }

    // Please note that this is one of the very few methods that aren't const.
    void set_crossover(const crossover_wrapper &);

    class const_iterator;
    friend class interpreter;

  private:
    hash_t hash() const;
    void pack(const locus &, std::vector<boost::uint8_t> *const) const;
    void tree(std::ostream &, const locus &, unsigned, const locus &) const;

  private:  // Private data members.
    // Crossover implementation can be changed/selected at runtime by this
    // polymorhic wrapper for function objects.
    // std::function can be easily bound to function pointers, member function
    // pointers, functors or anonymous (lambda) functions.
    crossover_wrapper crossover_;

    // Starting point of the active code in this individual (the best sequence
    // of genes is starting here).
    locus best_;

    const environment *env_;

    // This is the genome: the entire collection of genes (the entirety of an
    // organism's hereditary information).
    boost::multi_array<gene, 2> genome_;

    // Note that sintactically distinct (but logically equivalent) individuals
    // have the same signature. This is a very interesting  property, useful
    // for individual comparison, information retrieval, entropy calculation...
    mutable hash_t signature_;
  };  // class individual


  std::ostream &operator<<(std::ostream &, const individual &);

  individual one_point_crossover(const individual &, const individual &);
  individual two_point_crossover(const individual &, const individual &);
  individual uniform_crossover(const individual &, const individual &);


  class individual::const_iterator
  {
  public:
    explicit const_iterator(const individual &);

    ///
    /// \return \c false when the iterator reaches the end.
    ///
    bool operator()() const
    { return l_[0] < ind_.genome_.size() && !loci_.empty(); }

    const locus &operator++();

    ///
    /// \return reference to the current \a gene of the \a individual.
    ///
    const gene &operator*() const
    {
      assert(l_[0] < ind_.genome_.size());
      return ind_.genome_(l_);
    }

    ///
    /// \return pointer to the current \c gene of the \c individual.
    ///
    const gene *operator->() const
    {
      assert(l_[0] < ind_.genome_.size());
      return &ind_.genome_(l_);
    }

  private:
    const individual &ind_;
    locus               l_;
    std::set<locus>  loci_;
  };

  ///
  /// \example example1.cc
  /// Creates a random individual and shows its content.
  ///
  /// \example example3.cc
  /// Performs three types of crossover between two random individuals.
  ///
}  // namespace vita

#endif  // INDIVIDUAL_H
