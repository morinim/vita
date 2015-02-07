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

#if !defined(VITA_VARIANT_H)
#define      VITA_VARIANT_H

#include <stdexcept>
#include <type_traits>

namespace vita
{

// Implementation details, user shouldn't import this.
namespace detail
{
/// 0-based position of \a T in \a Ts.
template<class T, class... Ts> struct position;
template<int, class...> struct storage_ops;
template<class...> struct type_info;
} // namespace detail

///
/// \brief A variant library (like boost::variant) using C++11 variadic
///        templates and other C++11 features
///
/// The class implements a stack based "discriminated union" data structure.
/// Using variadic templates rather than a fixed large number of arguments with
/// defaults (as in Boost).
///
/// This is simple, keeps compile times down and provides more readable error
/// messages.
///
/// The class is not meant to be source compatible with Boost, but it provides
/// similar features.
///
/// \warning
/// All \a Ts... shall be object types and shall satisfy the requirements of
/// 'Destructible'.
///
template<class... Ts>
class variant
{
  static_assert(detail::type_info<Ts...>::no_reference_types,
                "Reference types are not permitted in variant");

  static_assert(detail::type_info<Ts...>::no_duplicates,
                "variant type arguments contain duplicate types");

public:
  template<class X> variant(const X &);
  ~variant();

  template<class X> void operator=(const X &);

  template<class X> X &get();
  template<class X> const X &get() const;

  template<class visitor> typename visitor::result_type visit(visitor &);

  int which() const;

private:
  variant() = delete;

  template<class X>
  void init(const X &x)
  {
    tag_ = detail::position<X, Ts...>::pos;
    new(storage_) X(x);
  }

  int tag_;

  // DO NOT REMOVE --> '+ 0' <--
  // That is to bypass a g++ v.4.8.x bug
  // (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55382)
  //
  // We could use std::aligned_union<0, Ts...>::type for the storage, but it
  // isn't well supported yet.
  alignas(detail::type_info<Ts...>::alignment + 0)
  char storage_[detail::type_info<Ts...>::size];
};

#include "variant.tcc"
}  // namespace vita

#endif  // Include guard
