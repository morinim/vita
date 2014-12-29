/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MATRIX_H)
#define      VITA_MATRIX_H

#include "kernel/locus.h"

namespace vita
{
///
/// \brief A bidimensional array
///
/// There are a lot of alternatives but this is *slim* and *fast*:
/// * std::vector<std::vector<T>> is slow;
/// * boost uBLAS and boost.MultiArray are good, general solutions but a bit
///   oversized for our needs;
///
/// The idea is to use a vector and translate the 2 dimensions to one
/// dimension (matrix::index() method). This way the whole thing is stored in
/// a single memory block instead of in several fragmented blocks for each
/// row.
///
/// \note
/// This class is based on \c std::vector. So although \c matrix<bool> will
/// work, you could prefer \c matrix<char> for performance reasons
/// (\c std::vector<bool> is a "peculiar" specialization).
///
  template<class T>
  class matrix
  {
  private:
    using values_t = std::vector<T>;

  public:  // Type alias
    using value_type = T;
    using reference = typename values_t::reference;
    using const_reference = typename values_t::const_reference;

  public:
    matrix();
    matrix(unsigned, unsigned);

    const_reference operator()(const locus &) const;
    reference operator()(const locus &);
    const_reference operator()(unsigned, unsigned) const;
    reference operator()(unsigned, unsigned);

    void fill(const T &);

    bool operator==(const matrix<T> &) const;

    unsigned rows() const;
    unsigned cols() const;

  public:  // Iterators
    using iterator = typename values_t::iterator;
    using const_iterator = typename values_t::const_iterator;

    iterator begin();
    const_iterator begin() const;
    const_iterator end() const;
    iterator end();

  public:   // Serialization
    bool load(std::istream &);
    bool save(std::ostream &) const;

  private:  // Private support functions
    unsigned size() const;
    unsigned index(unsigned, unsigned) const;

  private:  // Private data members
    values_t data_;

    unsigned cols_;
  };

#include "kernel/matrix.tcc"
}  // namespace vita

#endif  // Include guard
