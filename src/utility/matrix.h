/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MATRIX_H)
#define      VITA_MATRIX_H

#include "kernel/gp/locus.h"

namespace vita
{
///
/// A bidimensional dense matrix that is stored in row-major form.
///
/// There are a lot of alternatives but this is *slim* and *fast*:
/// - `std::vector<std::vector<T>>` is slow;
/// - boost *uBLAS* and *boost.MultiArray* are good, general solutions but
///   oversized for our needs.
///
/// The idea is to use a vector and translate the 2 dimensions to one
/// dimension (matrix::index() method). This way the whole thing is stored in
/// a contiguous memory block.
///
/// \note
/// This class is based on `std::vector`. So although `matrix<bool>` will
/// work, you could prefer `matrix<char>` for performance reasons
/// (`std::vector<bool>` is a *peculiar* specialization).
///
template<class T>
class matrix
{
public:
  // *** Type alias ***
  using values_t = std::vector<T>;
  using value_type = typename values_t::value_type;
  using reference = typename values_t::reference;
  using const_reference = typename values_t::const_reference;

  explicit matrix() : matrix(0, 0) {}
  explicit matrix(std::size_t, std::size_t);
  matrix(std::initializer_list<std::initializer_list<T>>);

  const_reference operator()(const locus &) const;
  reference operator()(const locus &);
  const_reference operator()(std::size_t, std::size_t) const;
  reference operator()(std::size_t, std::size_t);

  void fill(const T &);

  bool operator==(const matrix &) const;

  bool empty() const;
  std::size_t rows() const;
  std::size_t cols() const;

  matrix &operator+=(const matrix &);

  // *** Iterators ***
  using iterator = typename values_t::iterator;
  using const_iterator = typename values_t::const_iterator;

  iterator begin();
  const_iterator begin() const;
  const_iterator end() const;
  iterator end();

  // *** Serialization ***
  bool load(std::istream &);
  bool save(std::ostream &) const;

private:
  // *** Private support functions ***
  std::size_t size() const;
  std::size_t index(std::size_t, std::size_t) const;

  // *** Private data members ***
  values_t data_;

  std::size_t cols_;
};

template<class T> matrix<T> flip(matrix<T>, unsigned);
template<class T> matrix<T> fliplr(matrix<T>);
template<class T> matrix<T> flipud(matrix<T>);
template<class T> matrix<T> rot90(const matrix<T> &, unsigned = 1);
template<class T> matrix<T> transpose(const matrix<T> &);
template<class T> bool operator<(const matrix<T> &, const matrix<T> &);
template<class T> bool operator!=(const matrix<T> &, const matrix<T> &);

#include "utility/matrix.tcc"
}  // namespace vita

#endif  // include guard
