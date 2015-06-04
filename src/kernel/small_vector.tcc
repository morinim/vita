/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SMALL_VECTOR_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_SMALL_VECTOR_TCC)
#define      VITA_SMALL_VECTOR_TCC

/// Calls the destructor for objects in the range `[b, e)`.
template<class T>
void destroy_range(T *b, T *e)
{
  for (; b != e; ++b)
    b->~T();
}

/// Copies the range `[b, e)` onto the uninitialized memory starting with
/// `d`, constructing elements as needed. This is similar to
/// `std::uninitialized_copy` but doesn't handle exceptions.
template<class T, class It1, class It2>
void uninitialized_copy(It1 b, It1 e, It2 d)
{
  for (; b != e; ++b, ++d)
    ::new (d) T(*b);
}

/// Moves the range [b, e) onto the uninitialized memory starting with `d`,
/// constructing elements as needed.
template<class T, class It1, class It2>
void uninitialized_move(It1 b, It1 e, It2 d)
{
  for (; b != e; ++b, ++d)
    ::new (d) T(std::move(*b));
}


///
/// \brief small_vector with capacity equal to `S` and size equal to `n`
///
template<class T, std::size_t S>
small_vector<T, S>::small_vector(size_type n)
{
  if (n <= S)
  {
    data_ = local_storage_;
    size_ = data_ + n;
    capacity_ = data_ + S;
  }
  else  // n > S
  {
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    if (!std::is_pod<T>::value)
      for (std::size_t k(0); k < n; ++k)
        new (data_ + k) T();
  }

  assert(size() == n);
  assert(capacity() >= std::max(S, n));
}

template<class T, std::size_t S>
small_vector<T, S>::small_vector(size_type n, const T &x)
{
  if (n <= S)
  {
    data_ = local_storage_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::fill_n(begin(), n, x);
  }
  else
  {
    // We aren't using the array version of the new operator since it stores
    // additional information about the array size in the first few bytes of
    // the memory allocation.
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    // Similar to `std::uninitialized_fill_n(data_, n, x)` but we don't handle
    // exceptions.
    for (std::size_t k(0); k < n; ++k)
      new (data_ + k) T(x);
  }

  assert(size() == n);
  assert(capacity() >= std::max(S, n));
}

template<class T, std::size_t S>
small_vector<T, S>::small_vector(std::initializer_list<T> list)
{
  const auto n(list.size());

  if (n <= S)
  {
    data_ = local_storage_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::copy(list.begin(), list.end(), begin());
  }
  else
  {
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    uninitialized_copy<T>(list.begin(), list.end(), begin());
  }

  assert(size() == n);
  assert(capacity() >= std::max(S, n));
}

template<class T, std::size_t S>
small_vector<T, S>::small_vector(const small_vector &v)
{
  const auto n(v.size());

  if (n <= S)
  {
    data_ = local_storage_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::copy(v.begin(), v.end(), begin());
  }
  else
  {
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    uninitialized_copy<T>(v.begin(), v.end(), data_);
  }

  assert(size() == n);
  assert(capacity() >= std::max(S, n));
}

template<class T, std::size_t S>
small_vector<T, S>::small_vector(small_vector &&rhs)
{
  const auto n(rhs.size());

  if (n <= S)
  {
    data_ = local_storage_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::move(rhs.begin(), rhs.end(), begin());
  }
  else
  {
    data_ = rhs.data_;
    size_ = rhs.size_;
    capacity_ = rhs.capacity_;

    rhs.data_ = rhs.local_storage_;
    rhs.size_ = rhs.local_storage_;
    rhs.capacity_ = rhs.local_storage_ + S;
  }

  assert(size() == n);
  assert(capacity() >= std::max(S, n));
}

template<class T, std::size_t S>
small_vector<T, S> &small_vector<T, S>::operator=(const small_vector &rhs)
{
  if (this != &rhs)  // Avoid self-assignment
  {
    const auto n(rhs.size());

    const bool needs_memory(capacity() < n);

    if (needs_memory)
    {
      if (!local_storage_used())
        free_heap_memory();

      data_ = static_cast<T *>(::operator new(n * sizeof(T)));
      capacity_ = size_ = data_ + n;

      uninitialized_copy<T>(rhs.begin(), rhs.end(), begin());
    }
    else  // !needs_memory
    {
      if (!std::is_pod<T>::value)
      {
        if (!local_storage_used())
          destroy_range(begin() + n, end());
#if defined(VITA_SMALL_VECTOR_LOW_MEMORY)
        else
          std::fill(begin() + n, end(), T());
#endif
      }

      size_ = begin() + n;

      std::copy(rhs.begin(), rhs.end(), begin());
    }

    assert(size() == n);
    assert(capacity() >= std::max(S, n));
  }

  return *this;
}

template<class T, std::size_t S>
small_vector<T, S> &small_vector<T, S>::operator=(small_vector &&rhs)
{
  if (this != &rhs)  // Avoid self-assignment
  {
    const auto n(rhs.size());

    if (!local_storage_used())
      free_heap_memory();

    if (n <= S)
    {
      data_ = local_storage_;
      size_ = local_storage_ + n;
      capacity_ = local_storage_ + S;

      std::move(rhs.begin(), rhs.end(), begin());
    }
    else  // n > S
    {
#if defined(VITA_SMALL_VECTOR_LOW_MEMORY)
      if (local_storage_used())
        std::fill_n(begin(), S, T());
#endif

      data_ = rhs.data_;
      size_ = rhs.size_;
      capacity_ = rhs.capacity_;

      rhs.data_ = rhs.local_storage_;
      rhs.size_ = rhs.local_storage_;
      rhs.capacity_ = rhs.local_storage_ + S;
    }

    assert(size() == n);
    assert(capacity() >= std::max(S, n));
  }

  return *this;
}

template<class T, std::size_t S>
small_vector<T, S>::~small_vector()
{
  if (!local_storage_used())
    free_heap_memory();
}

template<class T, std::size_t S>
void small_vector<T, S>::clear()
{
  if (!local_storage_used())
    free_heap_memory();

  data_ = local_storage_;
  size_ = data_;
  capacity_ = data_ + S;

}

template<class T, std::size_t S>
void small_vector<T, S>::push_back(const T &x)
{
  if (size_ == capacity_)
  {
    const auto n_old(size());
    const auto n(n_old > 1 ? (3 * n_old) / 2 : n_old + 1);

    grow(n);
    size_ = data_ + n_old;
  }

  if (local_storage_used())
    *size_ = x;
  else
    new (size_) T(x);

  ++size_;
}

template<class T, std::size_t S>
template<class IT>
typename small_vector<T,S>::iterator small_vector<T, S>::append(IT b, IT e)
{
  const auto n(static_cast<size_type>(std::distance(b, e)));

  // Grow allocated space if needed.
  if (size_ + n > capacity_)
  {
    const auto n_old(size());
    const auto n_new(n_old + n);

    grow(n_new);
    size_ = data_ + n_old;
  }

  if (local_storage_used())
    std::copy(b, e, end());
  else
    uninitialized_copy<T>(b, e, end());

  size_ += n;

  return end();
}

template<class T, std::size_t S>
template<class IT>
typename small_vector<T,S>::iterator small_vector<T, S>::insert(
  iterator i, IT b, IT e)
{
  assert(i >= begin());
  assert(i <= end());

  if (i == end())  // Important special case for empty vector
    return append(b, e);

  // Convert iterator to index to avoid invalidating iterator after reserve().
  const auto insert_index(static_cast<size_type>(i - begin()));

  const auto n(static_cast<size_type>(std::distance(b, e)));

  // Ensure there is enough space.
  reserve(size() + n);

  // Uninvalidate the iterator.
  i = begin() + insert_index;

  // If there are more elements between the insertion point and the end of the
  // range than there are being inserted, we can use a simple approach to
  // insertion.  Since we already reserved space, we know that this won't
  // reallocate the vector.
  if (i + n <= end())
  {
    const auto old_end(end());

    append(std::move_iterator<iterator>(end() - n),
           std::move_iterator<iterator>(end()));

    // Copy the existing elements that get replaced.
    std::move_backward(i, old_end - n, old_end);

    std::copy(b, e, i);
    return i;
  }

  // Otherwise, we're inserting more elements than exist already, and we're
  // not inserting at the end.

  // Move over the elements that we're about to overwrite.
  const auto old_end(end());

  size_ += n;

  auto overwritten(old_end - i);

  uninitialized_move<T>(i, old_end, end() - overwritten);

  // Replace the overwritten part.
  for (auto j(i); overwritten; --overwritten, ++j, ++b)
    *j = *b;

  // Insert the non-overwritten middle part.
  uninitialized_copy<T>(b, e, old_end);

  return i;
}

///
/// \param[in] n new size of the container.
///
/// Resizes the container to contain `n` elements. If the current size is
/// - greater than `n`, the container is reduced to its first `n` elements;
/// - less than `n`, additional default-inserted elements are appended.
///
/// \note
/// Vector capacity is never reduced when resizing to smaller size because that
/// would invalidate all iterators, rather than only the ones that would be
/// invalidated by the equivalent sequence of `pop_back()` calls.
///
template<class T, std::size_t S>
void small_vector<T, S>::resize(size_type n)
{
  if (n <= capacity())
  {
    if (!std::is_pod<T>::value)
    {
      if (local_storage_used())
      {
        if (n >= size())
          std::fill(end(), begin() + n, T());
#if defined(VITA_SMALL_VECTOR_LOW_MEMORY)
        else
          std::fill(begin() + n, end(), T());
#endif
      }
      else  // !local_storage_used()
      {
        if (n < size())
          destroy_range(begin() + n, end());
        else
          for (auto k(size()); k < n; ++k)
            new (data_ + k) T();
      }
    }

    size_ = data_ + n;
    // Vector capacity isn't reduced.
  }
  else  // n > capacity()
  {
    const auto n_old(size());

    grow(n);
    size_ = capacity_;

    for (auto k(n_old); k < n; ++k)
      new (data_ + k) T();
  }

  assert(size() == n);
  assert(size() <= capacity());
}

///
/// \param[in] n new capacity of the container.
///
/// Increase the capacity of the container to a value that's grater or equal to
/// `n`. If `n` is greater than the current capacity(), new storeage is
/// allocated, otherwise the method does nothing.
///
/// If `n` is greater than capacity(), all iterators and references, including
/// the past-the-end iterator, are invalidated. Otherwise, no iterators or
/// references are invalidated.
///
template<class T, std::size_t S>
void small_vector<T, S>::reserve(size_type n)
{
  if (n > capacity())
  {
    const auto n_old(size());

    grow(n);
    size_ = data_ + n_old;
  }

  assert(size() <= capacity());
  assert(capacity() >= n);
}

template<class T, std::size_t S>
void small_vector<T, S>::free_heap_memory()
{
  assert(!local_storage_used());

  if (!std::is_pod<T>::value)
    destroy_range(begin(), end());

  ::operator delete(data_);
}

template<class T, std::size_t S>
void small_vector<T, S>::grow(size_type n)
{
  assert(capacity() < n);

  auto new_data(static_cast<T *>(::operator new(n * sizeof(T))));

  uninitialized_move<T>(begin(), end(), new_data);

  if (!local_storage_used())
    free_heap_memory();
#if defined(VITA_SMALL_VECTOR_LOW_MEMORY)
  else if (!std::is_pod<T>::value)
    std::fill(begin(), end(), T());
#endif

  data_ = new_data;
  capacity_ = data_ + n;
}

///
/// \brief Checks if the contents of `lhs` and `rhs` are equal
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` whether `lhs.size() == rhs.size()` and each element in `lhs`
///         compares equal with the element in `rhs` at the same position.
///
/// \relates small_vector
///
template<class T, std::size_t LS, std::size_t RS>
bool operator==(const small_vector<T, LS> &lhs, const small_vector<T, RS> &rhs)
{
  return lhs.size() == rhs.size() &&
         std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
}

///
/// \brief Checks if the contents of `lhs` and `rhs` aren't equal
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` whether `!(lhs == rhs)`.
///
/// \relates small_vector
///
template<class T, std::size_t LS, std::size_t RS>
bool operator!=(const small_vector<T, LS> &lhs, const small_vector<T, RS> &rhs)
{
  return !operator==(lhs, rhs);
}

///
/// \brief Compares the contents of `lhs` and `rhs` lexicographically
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` if the contents of `lhs` are lexicographically less than
///         the contents of `rhs`, `false` otherwise.
///
/// \relates small_vector
///
template<class T, std::size_t LS, std::size_t RS>
bool operator<(const small_vector<T, LS> &lhs, const small_vector<T, RS> &rhs)
{
  return std::lexicographical_compare(std::begin(lhs), std::end(lhs),
                                      std::begin(rhs), std::end(rhs));
}

///
/// \brief Compares the contents of `lhs` and `rhs` lexicographically
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` if the contents of `lhs` are lexicographically greater than
///         the contents of `rhs`, `false` otherwise.
///
/// \relates small_vector
///
template<class T, std::size_t LS, std::size_t RS>
bool operator>(const small_vector<T, LS> &lhs, const small_vector<T, RS> &rhs)
{
  return operator<(rhs, lhs);
}

///
/// \brief Compares the contents of `lhs` and `rhs` lexicographically
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` if the contents of the `lhs` are lexicographically greater
///         than or equal the contents of `rhs`, `false` otherwise.
///
/// \relates small_vector
///
template<class T, std::size_t LS, std::size_t RS>
bool operator>=(const small_vector<T, LS> &lhs, const small_vector<T, RS> &rhs)
{
  return !operator<(lhs, rhs);
}

///
/// \brief Compares the contents of `lhs` and `rhs` lexicographically
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` if the contents of the `lhs` are lexicographically less
///         than or equal the contents of `rhs`, `false` otherwise.
///
/// \relates small_vector
///
template<class T, std::size_t LS, std::size_t RS>
bool operator<=(const small_vector<T, LS> &lhs, const small_vector<T, RS> &rhs)
{
  return !operator>(lhs, rhs);
}

#endif  // Include guard
