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

#include <vector>

#include "kernel/locus.h"

namespace vita
{
  ///
  /// \brief A bidimensional array.
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
  template<class T>
  class matrix
  {
  public:
    matrix();
    matrix(unsigned, unsigned);

    const T &operator()(const locus &) const;
    T &operator()(const locus &);
    const T &operator()(unsigned, unsigned) const;
    T &operator()(unsigned, unsigned);

    void fill(const T &);

    bool operator==(const matrix<T> &) const;

    unsigned rows() const;
    unsigned cols() const;

  public:  // Iterators
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

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
    std::vector<T> data_;

    unsigned cols_;
  };

#include "kernel/matrix_inl.h"
}  // namespace vita

#endif  // Include guard
