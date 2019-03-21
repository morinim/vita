/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_MATRIX_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_MATRIX_TCC)
#define      VITA_MATRIX_TCC

///
/// Standard `rs` x `cs` matrix.
///
/// \param[in] rs number of rows
/// \param[in] cs number of columns
///
/// \note
/// Default values for `rs` and `cs` is `0` (i.e. uninitialized matrix,
/// sometimes we need it for performance).
///
template<class T>
matrix<T>::matrix(std::size_t rs, std::size_t cs) : data_(rs * cs), cols_(cs)
{
  Expects((rs && cs) || (!rs && !cs));
}

template<class T>
matrix<T>::matrix(std::initializer_list<std::initializer_list<T>> ll)
  : matrix(ll.size(), ll.size() ? ll.begin()->size() : 0)
{
  Expects(!ll.size() || (ll.size() && ll.begin()->size()));

  std::size_t i(0);

  for (const auto &row : ll)
    for (const auto &val : row)
      data_[i++] = val;

  Ensures(data_.size() == (ll.size() ? ll.size() * ll.begin()->size() : 0));
  Ensures(data_.size() == i);
}

///
/// \param[in] r row
/// \param[in] c column
/// \return      the index in the internal vector used to store the content of
///              the matrix
///
template<class T>
std::size_t matrix<T>::index(std::size_t r, std::size_t c) const
{
  assert(c < cols());

  return r * cols() + c;
}

///
/// \param[in] l a locus of the genome
/// \return      an element of the matrix
///
template<class T>
typename matrix<T>::const_reference matrix<T>::operator()(const locus &l) const
{
  return data_[index(l.index, l.category)];
}

///
/// \param[in] l a locus of the genome
/// \return      an element of the matrix
///
template<class T>
typename matrix<T>::reference matrix<T>::operator()(const locus &l)
{
  // DO NOT CHANGE THE RETURN TYPE WITH T (the method won't work for T == bool)
  return data_[index(l.index, l.category)];
}

///
/// \param[in] r row
/// \param[in] c column
/// \return      an element of the matrix
///
/// \see https://isocpp.org/wiki/faq/operator-overloading#matrix-array-of-array
///
template<class T>
typename matrix<T>::const_reference matrix<T>::operator()(std::size_t r,
                                                          std::size_t c) const
{
  return data_[index(r, c)];
}

///
/// \param[in] r row
/// \param[in] c column
/// \return      an element of the matrix
///
template<class T>
typename matrix<T>::reference matrix<T>::operator()(std::size_t r,
                                                    std::size_t c)
{
  // DO NOT CHANGE THE RETURN TYPE WITH T (won't work for `T == bool`)
  return data_[index(r, c)];
}

///
/// \return `true` if the matrix is empty (`cols() == 0`)
///
template<class T>
bool matrix<T>::empty() const
{
  return size() == 0;
}

///
/// \return number of elements of the matrix
///
template<class T>
std::size_t matrix<T>::size() const
{
  return data_.size();
}

///
/// \return number of rows of the matrix
///
template<class T>
std::size_t matrix<T>::rows() const
{
  return cols() ? data_.size() / cols() : 0;
}

///
/// \return number of columns of the matrix
///
template<class T>
std::size_t matrix<T>::cols() const
{
  return cols_;
}

///
/// \param[in] m second term of comparison
/// \return      `true` if `m` is equal to `*this`
///
template<class T>
bool matrix<T>::operator==(const matrix &m) const
{
  return cols() == m.cols() && data_ == m.data_;
}

///
/// Sets all the elements of the matrix to a specific value.
///
/// \param[in] v a value
///
template<class T>
void matrix<T>::fill(const T &v)
{
  std::fill(begin(), end(), v);
}

///
/// \return iterator to the first element of the matrix
///
template<class T>
typename matrix<T>::iterator matrix<T>::begin()
{
  return data_.begin();
}

///
/// \return constant iterator to the first element of the matrix
///
template<class T>
typename matrix<T>::const_iterator matrix<T>::begin() const
{
  return data_.begin();
}

///
/// \return iterator to the end (i.e. the element after the last element) of
///         the matrix
///
template<class T>
typename matrix<T>::const_iterator matrix<T>::end() const
{
  return data_.end();
}

///
/// \return iterator to the end (i.e. the element after the last element) of
///         the matrix
///
template<class T>
typename matrix<T>::iterator matrix<T>::end()
{
  return data_.end();
}

///
/// \param[in] m a matrix
/// \return      the sum of `this` and `m`
///
template<class T>
matrix<T> &matrix<T>::operator+=(const matrix<T> &m)
{
  Expects(m.cols() == cols());
  Expects(m.rows() == rows());

  for (std::size_t i(0); i < data_.size(); ++i)
    data_[i] += m.data_[i];

  return *this;
}

///
/// Saves the matrix on persistent storage.
///
/// \param[out] out output stream
/// \return         `true` on success
///
/// \note
/// The method is based on `operator<<` so it works for basic `T` only.
///
template<class T>
bool matrix<T>::save(std::ostream &out) const
{
  static_assert(std::is_integral<T>::value,
                "matrix::save doesn't support non-integral types");

  out << cols() << ' ' << rows() << '\n';

  for (const auto &e : data_)
    out << e << '\n';

  return out.good();
}

///
/// Loads the matrix from persistent storage.
///
/// \param[in] in input stream
/// \return       `true` on success
///
/// \note
/// * If the operation fails the object isn't modified.
/// * The method is based on `operator>>` so it works for basic `T` only.
///
template<class T>
bool matrix<T>::load(std::istream &in)
{
  static_assert(std::is_integral<T>::value,
                "matrix::load doesn't support non-integral types");

  decltype(cols_) cs;
  if (!(in >> cs))
    return false;

  decltype(cols_) rs;
  if (!(in >> rs))
    return false;

  decltype(data_) v(cs * rs);

  for (auto &e : v)
    if (!(in >> e))
      return false;

  cols_ = cs;
  data_ = v;

  assert(!empty() || (cols() == 0 && size() == 0));
  return true;
}

///
/// Flips matrix left to right.
///
/// \param[in] m input matrix
/// \return      flipped matrix
///
/// `B = fliplr(A)` returns `A` with its columns flipped in the left-right
/// direction (that is, about a vertical axis).
///
///  E.g. if `A = { {'a' 'b' 'c'}, {'d' 'e' 'f'}, {'g' 'h' 'i'} }` then
///  `fliplr(A) == { {'c', 'b', 'a'}, {'f', 'e', 'd'}, {'i', 'h', 'g'} }`.
///
/// \note Use flipup function to flip matrices in the vertical direction.
///
/// \relates matrix
///
template<class T> matrix<T> fliplr(matrix<T> m)
{
  const auto half(m.cols() / 2);

  for (std::size_t row(0); row < m.rows(); ++row)
    for (std::size_t col(0); col < half; ++col)
      std::swap(m(row, col), m(row, m.cols() - col - 1));

  return m;
}

///
/// Flips matrix up to down.
///
/// \param[in] m input matrix
/// \return      flipped matrix
///
/// `B = fliplr(A)` returns `A` with its rows flipped in the up-down direction
/// (that is, about a horizontal axis).
///
///  E.g. if `A = { {'a' 'b' 'c'}, {'d' 'e' 'f'}, {'g' 'h' 'i'} }` then
///  `fliplr(A) == { {'g', 'h', 'i'}, {'d', 'e', 'f'}, {'a', 'b', 'c'} }`.
///
/// \note Use fliplr function to flip matrices in the horizontal direction.
///
/// \relates matrix
///
template<class T> matrix<T> flipud(matrix<T> m)
{
  const auto half(m.rows() / 2);

  for (std::size_t row(0); row < half; ++row)
    for (std::size_t col(0); col < m.cols(); ++col)
      std::swap(m(row, col), m(m.rows() - row - 1, col));

  return m;
}

///
/// Flips order of elements.
///
/// `B = flip(A, dim)` reverses the order of the elements in `A` along
/// dimension `dim`. For example `flip(A, 1)` reverses the elements in each
/// column and `flip(A, 2)` reverses the elements in each row.
///
/// \note
/// - `flip(m, 1) == flipud(m)`;
/// - `flip(m, 2) == fliplr(m)`.
///
/// \relates matrix
///
template<class T> matrix<T> flip(matrix<T> m, unsigned dim)
{
  return dim == 2 ? fliplr(m) : flipud(m);
}

///
/// Rotates the matrix 90 degrees.
///
/// \param[in] m input matrix
/// \return      rotation constant
///
/// Rotates matrix `m` counterclockwise by `k * 90` degrees.
///
/// \relates matrix
///
template<class T> matrix<T> rot90(const matrix<T> &m, unsigned k)
{
  switch (k % 4)
  {
  case 0:
    return m;
  case 1:
    return flipud(transpose(m));
  case 2:
    return fliplr(flipud(m));
  default:
    assert(k == 3);
    return fliplr(transpose(m));
  }
}

///
/// Transposes a matrix.
///
/// \param[in] m a matrix
/// \return      the transpose of `m`
///
/// Interchanges the row and column index for each element of the matrix.
///
/// \relates matrix
///
template<class T> matrix<T> transpose(const matrix<T> &m)
{
  matrix<T> t(m.cols(), m.rows());

  for (std::size_t row(0); row < m.rows(); ++row)
    for (std::size_t col(0); col < m.cols(); ++col)
      t(col, row) = m(row, col);

  return t;
}

///
/// \param[in] lhs first matrix
/// \param[in] rhs second matrix
/// \return        `true` for different matrices
///
/// \relates matrix
///
template<class T>
bool operator!=(const matrix<T> &lhs, const matrix<T> &rhs)
{
  return !(lhs == rhs);
}

///
/// Lexicographically compares two matrices.
///
/// \param[in] lhs first matrix
/// \param[in] rhs second matrix
/// \return    `true` if `lhs` is lexicographically less than `rhs`
///
/// The function is based on `std::lexicographical_compare` and elements are
/// compared using `operator<`.
///
/// \note mainly useful for `std::set` compatibility.
///
/// \relates matrix
///
template<class T> bool operator<(const matrix<T> &lhs, const matrix<T> &rhs)
{
  return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                      rhs.begin(), rhs.end());
}

///
/// Prints the matrix on an output stream.
///
/// \param[out] o output stream
/// \param[in]  m a matrix
///
/// This is mainly used for debug purpose (Boost Test needs the operator to
/// report errors).
///
/// \relates matrix
///
template<class T>
std::ostream &operator<<(std::ostream &o, const matrix<T> &m)
{
  std::size_t i(0);

  for (const auto &e : m)
  {
    o << e;

    ++i;

    if (i % m.cols() == 0)
      o << '\n';
    else
      o << ' ';
  }

  return o;
}

#endif  // include guard
