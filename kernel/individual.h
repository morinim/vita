/**
 *
 *  \file individual.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
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
    individual mutation(unsigned * = 0) const;

    std::list<loc_t> blocks() const;
    individual destroy_block(unsigned) const;
    individual get_block(const loc_t &) const;
    individual replace(const symbol_ptr &, const std::vector<unsigned> &) const;
    individual replace(const symbol_ptr &, const std::vector<unsigned> &,
                       const loc_t &) const;

    individual generalize(unsigned, std::vector<loc_t> *const) const;

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
    const gene &operator[](const loc_t &l) const { return genome_(l); }

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
    void set(const loc_t &locus, const gene &g)
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
    void pack(const loc_t &, std::vector<boost::uint8_t> *const) const;
    void tree(std::ostream &, const loc_t &, unsigned, const loc_t &) const;

    // Crossover implementation can be changed/selected at runtime by this
    // polymorhic wrapper for function objects.
    // std::function can be easily bound to function pointers, member function
    // pointers, functors or anonymous (lambda) functions.
    crossover_wrapper crossover_;

    // Starting point of the active code in this individual (the best sequence
    // of genes is starting here).
    loc_t best_;

    const environment *env_;

    // This is the genome: the entire collection of genes (the entirety of an
    // organism's hereditary information).
    g_matrix genome_;

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

    const loc_t &operator++();

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
    loc_t               l_;
    std::set<loc_t>  loci_;
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
