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

///
/// \brief small_vector with capacity equal to `S` and size equal to `n`
///
template<class T, std::size_t S>
small_vector<T, S>::small_vector(std::size_t n)
{
  if (n <= S)
  {
    data_ = data_small_;
    size_ = data_ + n;
    capacity_ = data_ + S;
  }
  else  // n > S
  {
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    // We aren't using the array version of the new operator since, in some
    // (many?) implementation, it stores information about the array size in
    // the first few bytes of the memory allocation.
    if (!std::is_pod<T>::value)
      for (std::size_t k(0); k < n; ++k)
        new (data_ + k) T();
  }

  assert(size() == n);
  assert(capacity() >= std::max(S, n));
}

template<class T, std::size_t S>
small_vector<T, S>::small_vector(std::size_t n, const T &x)
{
  if (n <= S)
  {
    data_ = data_small_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::fill_n(begin(), n, x);
  }
  else
  {
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
    data_ = data_small_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::copy(list.begin(), list.end(), begin());
  }
  else
  {
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    uninitialized_copy(list.begin(), list.end(), begin());
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
    data_ = data_small_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::copy(v.begin(), v.end(), begin());
  }
  else
  {
    data_ = static_cast<T *>(::operator new(n * sizeof(T)));
    capacity_ = size_ = data_ + n;

    uninitialized_copy(v.begin(), v.end(), data_);
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
    data_ = data_small_;
    size_ = data_ + n;
    capacity_ = data_ + S;

    std::move(rhs.begin(), rhs.end(), begin());
  }
  else
  {
    data_ = rhs.data_;
    size_ = rhs.size_;
    capacity_ = rhs.capacity_;

    rhs.data_ = rhs.data_small_;
    rhs.size_ = rhs.data_small_;
    rhs.capacity_ = rhs.data_small_ + S;
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
      if (!is_data_small_used())
        free_heap_memory();

      data_ = static_cast<T *>(::operator new(n * sizeof(T)));
      capacity_ = size_ = data_ + n;

      uninitialized_copy(rhs.begin(), rhs.end(), begin());
    }
    else  // !needs_memory
    {
      if (!std::is_pod<T>::value)
      {
        if (is_data_small_used())
          std::fill(begin() + n, end(), T());
        else
          destroy_range(begin() + n, end());
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

    if (n <= S)
    {
      if (!is_data_small_used())
        free_heap_memory();

      data_ = data_small_;
      size_ = data_small_ + n;
      capacity_ = data_small_ + S;

      std::move(rhs.begin(), rhs.end(), begin());
    }
    else  // n > S
    {
      if (is_data_small_used())
        std::fill_n(begin(), S, T());
      else
        free_heap_memory();

      data_ = rhs.data_;
      size_ = rhs.size_;
      capacity_ = rhs.capacity_;

      rhs.data_ = rhs.data_small_;
      rhs.size_ = rhs.data_small_;
      rhs.capacity_ = rhs.data_small_ + S;
    }

    assert(size() == n);
    assert(capacity() >= std::max(S, n));
  }

  return *this;
}

template<class T, std::size_t S>
small_vector<T, S>::~small_vector()
{
  if (!is_data_small_used())
    free_heap_memory();
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

  if (is_data_small_used())
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

  if (is_data_small_used())
    std::copy(b, e, end());
  else
    uninitialized_copy(b, e, end());

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

  uninitialized_move(i, old_end, end() - overwritten);

  // Replace the overwritten part.
  for (auto j(i); overwritten; --overwritten, ++j, ++b)
    *j = *b;

  // Insert the non-overwritten middle part.
  uninitialized_copy(b, e, old_end);

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
void small_vector<T, S>::resize(std::size_t n)
{
  if (n <= capacity())
  {
    if (!std::is_pod<T>::value)
    {
      if (is_data_small_used())
      {
        if (n < size())
          std::fill(begin() + n, end(), T());
        else
          std::fill(end(), begin() + n, T());
      }
      else  // !is_data_small_used()
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
void small_vector<T, S>::reserve(std::size_t n)
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
void small_vector<T, S>::free_all_memory()
{
  if (is_data_small_used())
  {
    if (!std::is_pod<T>::value)
      std::fill(begin(), end(), T());
  }
  else
    free_heap_memory();
}

template<class T, std::size_t S>
void small_vector<T, S>::grow(size_type n)
{
  assert(capacity() < n);

  auto new_data(static_cast<T *>(::operator new(n * sizeof(T))));

  uninitialized_move(begin(), end(), new_data);

  free_all_memory();

  data_ = new_data;
  capacity_ = data_ + n;
}

#endif  // Include guard
