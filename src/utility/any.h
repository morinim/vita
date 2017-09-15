/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ANY_H)
#define      VITA_ANY_H

#include "kernel/common.h"

#include <typeinfo>

namespace vita
{

namespace detail
{
namespace any_
{
struct empty {};
struct fxn_ptr_table;
template<bool> struct fxns;
template<class> struct get_table;
}  // namespace any_
}  // namespace detail

///
/// The exception thrown in the event of a failed any_cast of an any value.
///
struct bad_any_cast : std::bad_cast
{
  bad_any_cast(const std::type_info &src, const std::type_info &dest)
    : from(src.name()), to(dest.name())
  {}

  const char *what() const noexcept override { return "bad any cast"; }

  const char *from;
  const char *to;
};

///
/// A `std::any` equivalent with small buffer optimization and support for the
/// streaming operators.
///
/// This class is derived from `boost::spirit::hold_any` (which is built based
/// on the any class published in
/// <http://www.codeproject.com/cpp/dynamic_typing.asp>).
///
class any
{
public:
  // Constructors / destructor.
  template<class T> explicit any(const T &);
  any() noexcept;
  any(const any &);

  /// Move constructor that moves content of `x` into new instance and leaves
  /// `x` empty.
  ///
  /// \post `x->empty()`
  any(any &&x) noexcept : any() { swap(x); }

  ~any();

  // Assignment operators.
  template<class T> any &operator=(T &&);

  /// Copies content of rhs into current instance.
  ///
  /// Discards previous content, so that the new content is equivalent in both
  /// type and value to the content of `rhs`, or empty if `rhs.empty()`.
  ///
  /// \post `rhs.empty()`
  any &operator=(const any &rhs) { return assign(rhs); }

  /// Assigns by moving the state of `rhs`.
  ///
  /// \note
  /// `rhs` is left in a valid but unspecified state after the assignment.
  any &operator=(any &&rhs) noexcept
  { return any(std::move(rhs)).swap(*this); }

  // Utility functions
  /// Exchange of the contents of `*this` and `rhs`.
  any &swap(any &rhs) noexcept
  {
    std::swap(table, rhs.table);
    std::swap(object, rhs.object);
    return *this;
  }

  const std::type_info &type() const noexcept;

  bool has_value() const noexcept;

  void reset() noexcept;

  // Types
  template<class T> friend T *any_cast(any *) noexcept;

private:
  template<class T> explicit any(const T &, std::true_type *);
  template<class T> explicit any(const T &, std::false_type *);

  // Assignment
  template<class T> any &assign(const T &);
  template<class T> any &assign(const T &, std::true_type *);
  template<class T> any &assign(const T &, std::false_type *);
  any &assign(const any &);

  // Private data members
  detail::any_::fxn_ptr_table *table;

  void *object = nullptr;
};  // class any

template<class T> T to(const any &);

#include "utility/any.tcc"
}  // namespace vita

#endif  // include guard
