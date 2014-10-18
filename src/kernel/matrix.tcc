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

#if !defined(VITA_MATRIX_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_MATRIX_TCC)
#define      VITA_MATRIX_TCC

///
/// \brief Uninitialized matrix... can be dangerous but sometimes we need it
///        for performance.
///
template<class T>
matrix<T>::matrix() : data_(), cols_(0)
{
}

///
/// \param[in] rs number of rows.
/// \param[in] cs number of columns.
///
/// \brief Standard \a rows x \a cols matrix. Entries aren't initialized.
///
template<class T>
matrix<T>::matrix(unsigned rs, unsigned cs) : data_(rs * cs), cols_(cs)
{
}

///
/// \param[in] r row.
/// \param[in] c column.
/// \return index in \a data_ vector.
///
/// \brief From (c, r) to an index in \a data_ vector.
///
template<class T>
unsigned matrix<T>::index(unsigned r, unsigned c) const
{
  assert(c < cols_);

  return r * cols_ + c;
}

///
/// \param[in] l a locus of the genome.
/// \return an element of the matrix.
///
template<class T>
const T &matrix<T>::operator()(const locus &l) const
{
  return data_[index(l.index, l.category)];
}

///
/// \param[in] l a locus of the genome.
/// \return an element of the matrix.
///
template<class T>
T &matrix<T>::operator()(const locus &l)
{
  return data_[index(l.index, l.category)];
}

///
/// \param[in] r row.
/// \param[in] c column.
/// \return an element of the matrix.
///
template<class T>
const T &matrix<T>::operator()(unsigned r, unsigned c) const
{
  return data_[index(r, c)];
}

///
/// \param[in] r row.
/// \param[in] c column.
/// \return an element of the matrix.
///
template<class T>
T &matrix<T>::operator()(unsigned r, unsigned c)
{
  return data_[index(r, c)];
}

///
/// \return number of elements of the matrix.
///
template<class T>
unsigned matrix<T>::size() const
{
  return static_cast<unsigned>(data_.size());
}

///
/// \return number of rows of the matrix.
///
template<class T>
unsigned matrix<T>::rows() const
{
  return static_cast<unsigned>(data_.size() / cols_);
}

///
/// \return number of columns of the matrix.
///
template<class T>
unsigned matrix<T>::cols() const
{
  return cols_;
}

///
/// \param[in] m second term of comparison.
/// \return \c true if \a m is equal to \c *this.
///
template<class T>
bool matrix<T>::operator==(const matrix<T> &m) const
{
  return cols() == m.cols() && data_ == m.data_;
}

///
/// \param[in] v a value.
///
/// Sets the elements of the matrix to \a v.
///
template<class T>
void matrix<T>::fill(const T &v)
{
  std::fill(begin(), end(), v);
}

///
/// \return iterator to the first element of the matrix.
///
template<class T>
typename matrix<T>::iterator matrix<T>::begin()
{
  return data_.begin();
}

///
/// \return constant iterator to the first element of the matrix.
///
template<class T>
typename matrix<T>::const_iterator matrix<T>::begin() const
{
  return data_.begin();
}

///
/// \return iterator to the end (i.e. the element after the last element) of
///         the matrix.
///
template<class T>
typename matrix<T>::const_iterator matrix<T>::end() const
{
  return data_.end();
}

///
/// \return iterator to the end (i.e. the element after the last element) of
///         the matrix.
///
template<class T>
typename matrix<T>::iterator matrix<T>::end()
{
  return data_.end();
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the matrix on persistent storage.
///
/// \note
/// The method is based on operator<< so it works for basic \a T only.
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
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the matrix from persistent storage.
///
/// \note
/// * If the operation fails the object isn't modified.
/// * The method is based on operator>> so it works for basic \a T only.
///
template<class T>
bool matrix<T>::load(std::istream &in)
{
  static_assert(std::is_integral<T>::value,
                "matrix::load doesn't support non-integral types");

  decltype(cols_) cs;
  if (!(in >> cs) || !cs)
    return false;

  decltype(cols_) rs;
  if (!(in >> rs) || !rs)
    return false;

  decltype(data_) v(cs * rs);

  for (auto &e : v)
    if (!(in >> e))
      return false;

  cols_ = cs;
  data_ = v;

  return true;
}

///
/// \param[out] o output stream
/// \param[in] m a matrix
///
/// Prints \a m on the output stream. This is mainly used for debug purpose
/// (boost test needs the operator to report errors).
///
template<class T>
std::ostream &operator<<(std::ostream &o, const matrix<T> &m)
{
  unsigned i(0);

  for (const auto &e : m)
  {
    o << e << (i && (i % m.cols()) == 0 ? '\n' : ' ');

    ++i;
  }

  return o;
}

#endif  // Include guard
