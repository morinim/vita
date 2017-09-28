/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SMALL_VECTOR_H)
#define      VITA_SMALL_VECTOR_H

#include <cassert>
#include <initializer_list>
#include <memory>

namespace vita
{

///
/// A simple class that looks and smells just like `std::vector<T>`.
///
/// \tparam T type of the elements
/// \tparam S typical maximum size of the vector
///
/// `small_vector` supports efficient iteration, lays out elements in memory
/// order (so you can do pointer arithmetic between elements), supports
/// efficient `push_back` / `pop_back` operations, efficient random access to
/// its elements...
///
/// The advantage of small_vector is that it allocates space for some number of
/// elements (`S`) in the object itself. Because of this, if the small_vector
/// is dynamically smaller than `S`, no `malloc` is performed. This can be a
/// big win in cases where the malloc/free call is far more expensive than the
/// code that fiddles around with the elements.
///
/// This is good for vectors that are "usually small" (e.g. the number of
/// predecessors/successors of a block is usually less than 8). On the other
/// hand, this makes the size of the small_vector itself large, so you don't
/// want to allocate lots of them (doing so will waste a lot of space). As
/// such, small vectors are most useful when on the stack.
///
/// \warning Note that this doesn't attempt to be exception safe.
///
template<class T, std::size_t S>
class small_vector
{
public:
  static_assert(S >= 1, "small_vector requires a positive size");

  // *** Type aliases *** (for playing nicely with STL)
  using value_type = T;

  using iterator = value_type *;
  using const_iterator = const value_type *;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using pointer = value_type *;
  using const_pointer = const value_type *;

  using reference = value_type &;
  using const_reference = const value_type &;

  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  // *** Constructors / destructor ***
  explicit small_vector(size_type = 0);
  small_vector(size_type, const T &);
  small_vector(std::initializer_list<T>);
  small_vector(const small_vector &);
  small_vector(small_vector &&);

  ~small_vector();

  small_vector &operator=(const small_vector &);
  small_vector &operator=(small_vector &&);

  void clear();

  /// \param[in] pos position of the element to return
  /// \return        a reference to the elemente at specified location `pos`
  /// \remark No bounds checking is performed.
  const_reference operator[](size_type pos) const
  {
    assert(pos < size());
    return cbegin()[pos];
  }

  /// \param[in] pos position of the element to return
  /// \return        a constant reference to the element at specified location
  ///                (`pos`)
  /// \remark No bounds checking is performed.
  reference operator[](size_type pos)
  {
    assert(pos < size());
    return begin()[pos];
  }

  /// \return a pointer to the vector's buffer, even if `empty()`
  pointer data() { return data_; }
  /// \return a pointer to the vector's buffer, even if `empty()`
  const_pointer data() const { return data_; }

  /// \return an iterator to the first element of the container
  /// \note
  /// if the container is empty, the returned iterator is equal to `end()`.
  iterator begin() { return data_; }
  /// \return an iterator to the element following the last element of the
  ///         container
  /// \warning
  /// This element acts as a placeholder; attempting to access it results in
  /// undefined behaviour.
  iterator end() { return size_; }

  const_iterator cbegin() const { return data_; }
  const_iterator cend() const { return size_; }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

  /// \return a reverse iterator to the first element of the reversed
  ///         container. It corresponds to the last element of the non-reversed
  ///         container
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /// \return a reverse itarator to the element following the last element of
  ///         of the reversed container. It corresponds to the element
  ///         preceding the first element of the non-reversed container
  /// \warning
  /// This element acts as a placeholder, attempting to access it results in
  /// undefined behaviour.
  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const {return const_reverse_iterator(end());}
  const_reverse_iterator rend() const {return const_reverse_iterator(begin());}

  /// \return the number of elements in the container
  size_type size() const
  {
    assert(end() >= begin());
    return static_cast<size_type>(end() - begin());
  }

  /// \return the capacity of the currently allocated storage
  size_type capacity() const
  {
    assert(capacity_ >= data_);
    return static_cast<size_type>(capacity_ - begin());
  }

  /// \return the maximum number of elements the container is able to hold
  ///         due to system or library implementation limitations
  /// \warning
  /// At runtime the size of the container may be limited to a value smaller
  /// than `max_size()` by the amount of RAM available.
  size_type max_size() const { return static_cast<size_type>(-1); }

  /// \return `true` if the container is empty, `false` otherwise
  bool empty() const { return end() == begin(); }

  // \return a reference to the first element in the container
  // \warning Calling `front` on an empty container is undefined.
  reference front()
  {
    assert(!empty());
    return begin()[0];
  }
  // \return a reference to the first element in the container
  // \warning Calling `front` on an empty container is undefined.
  const_reference front() const
  {
    assert(!empty());
    return cbegin()[0];
  }

  // \return a reference to the last element in the container
  // \warning Calling `back` on an empty container is undefined.
  reference back()
  {
    assert(!empty());
    return end()[-1];
  }
  // \return a reference to the last element in the container
  // \warning Calling `back` on an empty container is undefined.
  const_reference back() const
  {
    assert(!empty());
    return end()[-1];
  }

  void resize(size_type);
  void reserve(size_type);

  void push_back(const T &);
  template<class... Args> void emplace_back(Args &&...);

  template<class IT> iterator insert(iterator, IT, IT);

private:
  // *** Support methods ***
  bool local_storage_used() const { return data_ == local_storage_; }

  void free_heap_memory();
  void grow();
  void grow(size_type);

  template<class IT> iterator append(IT, IT);

  // *** Private data members ***
  // Always point to the beginning of the vector. It points to some memory on
  // the heap when small size optimization is not used and points to
  // `local_storage_` when small size optimization is used.
  T *data_;

  T *size_;

  // The capacity of the vector is always `>= S` whether small size
  // optimization is in use (in this case the capacity is equal to `S`) or not.
  T *capacity_;

  // Objects contained in `local_storage_` are never destructed; if the macro
  // `VITA_SMALL_VECTOR_LOW_MEMORY` is defined, they're reinitialized to `T()`
  // when not used anymore. Objects on the heap are destructed if:
  // - they aren't plain old data;
  // - aren't used anymore.
  T local_storage_[S];
};

#include "utility/small_vector.tcc"
}  // namespace vita

#endif  // include guard
