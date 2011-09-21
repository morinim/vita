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
#include <cstring>
#include <iomanip>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "kernel/gene.h"

namespace vita
{
  class environment;

  ///
  /// A single member of a \a population. Each individual contains a genome
  /// which represents a possible solution to the task being tackled (i.e. a
  /// point in the search space).
  ///
  class individual
  {
  public:
    individual(const environment &, bool);

    void dump(std::ostream &) const;
    void graphviz(std::ostream &, const std::string & = "") const;
    void in_line(std::ostream &) const;
    void list(std::ostream &) const;
    void tree(std::ostream &) const;

    unsigned mutation();
    individual &uniform_cross(const individual &);
    individual cross1(const individual &) const;
    individual cross2(const individual &) const;

    std::list<unsigned> blocks() const;
    individual &destroy_block(unsigned);
    individual get_block(unsigned) const;
    individual replace(symbol_ptr, const std::vector<locus_t> &) const;
    individual replace(symbol_ptr, const std::vector<locus_t> &,
                       unsigned) const;

    individual generalize(std::size_t, std::vector<locus_t> *,
                          std::vector<symbol_t> *) const;

    individual compact(unsigned * = 0) const;
    individual optimize(unsigned * = 0, unsigned * = 0) const;

    bool operator==(const individual &) const;
    bool operator!=(const individual &x) const { return !(*this == x); }
    unsigned distance(const individual &) const;

    ///
    /// \param[out] p packed version (byte stream) of \c this individual.
    ///
    void pack(std::vector<boost::uint8_t> *const p) const { pack(p, best_); }

    bool check() const;

    ///
    /// \param[in] i index of the \c gene of the \c individual.
    /// \return the i-th \c gene of the \c individual.
    ///
    const gene &operator[](unsigned i) const { return code_[i]; }

    ///
    /// \return the total size of the individual (effective size + introns).
    ///
    /// The size is constant for any individual (it's choosen at initialization
    /// time).
    /// \see eff_size
    ///
    unsigned size() const { return code_.size(); }

    unsigned eff_size() const;

    symbol_t type() const;

    class const_iterator;
    friend class const_iterator;
    friend class    interpreter;

  private:
    void pack(std::vector<boost::uint8_t> *const, unsigned) const;
    void tree(std::ostream &, unsigned, unsigned, unsigned) const;
    unsigned unpack(const std::vector<boost::uint8_t> &, unsigned);

    // Active code in this individual (the best sequence of genes is starting
    // here).
    unsigned          best_;

    const environment *env_;

    // This is the genome: the entire collection of genes.
    std::vector<gene> code_;
  };

  std::ostream & operator<<(std::ostream &, const individual &);

  class individual::const_iterator
  {
  public:
    explicit const_iterator(const individual &);

    ///
    /// \return \c false when the iterator reaches the end.
    ///
    bool operator()() const
    { return l_ < ind_.code_.size() && !lines_.empty(); }

    unsigned operator++();

    ///
    /// \return reference to the current \a gene of the \a individual.
    ///
    const gene &operator*() const
    {
      assert(l_ < ind_.code_.size());
      return ind_.code_[l_];
    }

    ///
    /// \return pointer to the current \c gene of the \c individual.
    ///
    const gene *operator->() const
    {
      assert(l_ < ind_.code_.size());
      return &ind_.code_[l_];
    }

  private:
    const individual    &ind_;
    unsigned               l_;
    std::set<unsigned> lines_;
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
