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
 *
 *  Original idea by Kenneth Micklas
 *  (<https://github.com/kmicklas/variadic-variant>).
 *  Code used under MPL2 licence with author's permission (2015-02-03).
 */

#if !defined(VITA_SMALL_VECTOR_H)
#define      VITA_SMALL_VECTOR_H

#include <cassert>
#include <initializer_list>
#include <new>

namespace vita
{

///
/// \brief A simple class that looks and smells just like `std::vector<T>`
///
/// \tparam T type of the elements
/// \tparam S typical maximum size of the vector
///
/// `small_vector` supports efficient iteration, lays out elements in memory
/// order (so you can do pointer arithmetic between elements), supports
/// efficient push_back/pop_back operations, supports efficient random access
/// to its elements...
///
/// The advantage of small_vector is that it allocates space for some number of
/// elements (`S`) in the object itself. Because of this, if the small_vector
/// is dynamically smaller than `S`, no malloc is performed. This can be a big
/// win in cases where the malloc/free call is far more expensive than the code
/// that fiddles around with the elements.
///
/// This is good for vectors that are "usually small" (e.g. the number of
/// predecessors/successors of a block is usually less than 8). On the other
/// hand, this makes the size of the small_vector itself large, so you don't
/// want to allocate lots of them (doing so will waste a lot of space). As
/// such, small vectors are most useful when on the stack.
///
template<class T, std::size_t S = 0>
class small_vector
{
public:  // Type aliases
  using value_type = T;
  using size_type = std::size_t;

  using iterator = value_type *;
  using const_iterator = const value_type *;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using pointer = value_type *;
  using const_pointer = const value_type *;

  using reference = value_type &;
  using const_reference = const value_type &;

public:
  explicit small_vector(std::size_t = 0);
  small_vector(std::size_t, const T &);
  small_vector(std::initializer_list<T>);
  small_vector(const small_vector &);
  small_vector(small_vector &&);

  ~small_vector();

  small_vector &operator=(const small_vector &);
  small_vector &operator=(small_vector &&);

  const_reference operator[](std::size_t k) const
  {
    assert(k < size());
    return cbegin()[k];
  }

  reference operator[](std::size_t k)
  {
    assert(k < size());
    return begin()[k];
  }

  /// Return a pointer to the vector's buffer, even if empty().
  pointer data() { return data_; }

  /// Return a pointer to the vector's buffer, even if empty().
  const_pointer data() const { return data_; }

  iterator begin() { return data_; }
  iterator end() { return size_; }

  const_iterator cbegin() const { return data_; }
  const_iterator cend() const { return size_; }
  const_iterator begin() const { return cbegin(); }
  const_iterator end() const { return cend(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const{ return const_reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin());}

  size_type size() const
  {
    assert(end() >= begin());
    return static_cast<size_type>(end() - begin());
  }

  size_type capacity() const
  {
    assert(capacity_ >= data_);
    return static_cast<size_type>(capacity_ - begin());
  }

  size_type max_size() const { return static_cast<size_type>(-1); }

  bool empty() const { return end() == begin(); }

  reference front()
  {
    assert(!empty());
    return begin()[0];
  }
  const_reference front() const
  {
    assert(!empty());
    return cbegin()[0];
  }

  reference back()
  {
    assert(!empty());
    return end()[-1];
  }
  const_reference back() const
  {
    assert(!empty());
    return end()[-1];
  }

  void resize(std::size_t);
  void reserve(std::size_t);

  void push_back(const T &);
  template<class IT> iterator insert(iterator, IT, IT);

private:  // Support methods
  static void destroy_range(T *b, T *e)
  {
    while (b != e)
    {
      --e;
      e->~T();
    }
  }

  /// Copy the range `[I, E)` onto the uninitialized memory starting with
  /// `d`, constructing elements as needed. This is similar to
  /// `std::uninitialized_copy` but doesn't handle exceptions.
  template<class It1, class It2>
  static void uninitialized_copy(It1 b, It1 e, It2 d)
  {
    for (; b != e; ++b, ++d)
      ::new (d) T(*b);
  }

  /// Move the range [I, E) onto the uninitialized memory starting with
  /// `dest`, constructing elements as needed.
  template<class It1, class It2>
  static void uninitialized_move(It1 b, It1 e, It2 d)
  {
    for (; b != e; ++b, ++d)
      ::new (d) T(std::move(*b));
  }

  bool is_data_small_used() const { return data_ == data_small_; }

  void free_heap_memory();
  void grow(size_type);

  template<class IT> iterator append(IT, IT);

private:  // Private data members
  // Always point to the beginning of the vector. It points to some memory on
  // the heap when small size optimization is not used and points to
  // `data_small_` when small size optimization is used.
  T *data_;

  T *size_;

  // The capacity of the vector is always `>= S` whether small size
  // optimization is in use (in this case the capacity is equal to `S`) or not.
  T *capacity_;

  // Objects on `data_small_` are never destructed but are reinitialized to T()
  // when not used anymore. Objects on the heap are destructed if:
  // - they aren't plain old data;
  // - aren't used anymore.
  T data_small_[S > 0 ? S : 1];
};

#include "kernel/staging/small_vector.tcc"
}  // namespace vita

#endif  // Include guard
