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
  /// dimension (matrix::index() method).
  ///
  template<class T>
  class matrix
  {
  public:
    matrix();
    matrix(size_t, size_t);

    const T &operator()(const locus &) const;
    T &operator()(const locus &);
    const T &operator()(size_t, size_t) const;
    T &operator()(size_t, size_t);

    void fill(const T &);

    bool operator==(const matrix<T> &) const;

    size_t rows() const;
    size_t cols() const;

    typename std::vector<T>::iterator begin();
    typename std::vector<T>::const_iterator begin() const;
    typename std::vector<T>::const_iterator end() const;

  private:  // Private support functions.
    size_t size() const;
    size_t index(size_t, size_t) const;

  private:  // Private data members.
    std::vector<T> data_;

    size_t rows_;
    size_t cols_;
  };

  ///
  /// \brief Uninitialized matrix... can be dangerous but sometimes we need it
  ///        for performance.
  ///
  template<class T>
  inline
  matrix<T>::matrix()
#if !defined(NDEBUG)
    : rows_(0), cols_(0)
#endif
  {
  }

  ///
  /// \param[in] rows number of rows.
  /// \param[in] cols number of columns.
  ///
  /// \brief Standard \a rows x \a cols matrix. Entries aren't initialized.
  ///
  template<class T>
  matrix<T>::matrix(size_t rows, size_t cols) :
    data_(rows * cols), rows_(rows), cols_(cols)
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
  inline
  size_t matrix<T>::index(size_t r, size_t c) const
  {
    assert(r < rows_);
    assert(c < cols_);

    return r * cols_ + c;
  }

  ///
  /// \param[in] l a locus of the genome.
  /// \return an element of the matrix.
  ///
  template<class T>
  inline
  const T &matrix<T>::operator()(const locus &l) const
  {
    return data_[index(l.index, l.category)];
  }

  ///
  /// \param[in] l a locus of the genome.
  /// \return an element of the matrix.
  ///
  template<class T>
  inline
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
  inline
  const T &matrix<T>::operator()(size_t r, size_t c) const
  {
    return data_[index(r, c)];
  }

  ///
  /// \param[in] r row.
  /// \param[in] c column.
  /// \return an element of the matrix.
  ///
  template<class T>
  T &matrix<T>::operator()(size_t r, size_t c)
  {
    return data_[index(r, c)];
  }

  ///
  /// \return number of elements of the matrix.
  ///
  template<class T>
  inline
  size_t matrix<T>::size() const
  {
    return rows_ * cols_;
  }

  ///
  /// \return number of rows of the matrix.
  ///
  template<class T>
  inline
  size_t matrix<T>::rows() const
  {
    return rows_;
  }

  ///
  /// \return number of columns of the matrix.
  ///
  template<class T>
  inline
  size_t matrix<T>::cols() const
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
    return cols() == m.cols() && rows() == m.rows() && data_ == m.data_;
  }

  ///
  /// \param[in] v a value.
  ///
  /// \brief Sets the elements of the matrix to \a v.
  ///
  template<class T>
  void matrix<T>::fill(const T &v)
  {
    const size_t sup(size());
    for (size_t i(0); i < sup; ++i)
      data_[i] = v;
  }

  ///
  /// \return iterator to the first element of the matrix.
  ///
  template<class T>
  inline
  typename std::vector<T>::iterator matrix<T>::begin()
  {
    return data_.begin();
  }

  ///
  /// \return constant iterator to the first element of the matrix.
  ///
  template<class T>
  inline
  typename std::vector<T>::const_iterator matrix<T>::begin() const
  {
    return data_.begin();
  }

  ///
  /// \return iterator to the end (i.e. the element after the last element) of
  ///         the matrix.
  ///
  template<class T>
  inline
  typename std::vector<T>::const_iterator matrix<T>::end() const
  {
    return data_.end();
  }
}  // namespace vita

#endif  // Include guard
