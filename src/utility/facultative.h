/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_FACULTATIVE_H)
#define      VITA_FACULTATIVE_H

#include <cmath>
#include <limits>
#include <sstream>
#include <type_traits>

namespace vita
{

///
/// Contains predefined policies for the `facultative` class.
///
/// A policy class must define two static member functions
/// - the first function creates a sentinel value;
/// - the second one checks the current value for being special.
///
namespace facultative_ns
{

struct default_tag{};

template<class T, T V>
struct value
{
  using value_type = T;
  static constexpr T empty_value() noexcept { return V; }
  static constexpr bool is_empty(T v) { return v == V; }
};

template<class T>
struct nan
{
  using value_type = T;
  static constexpr T empty_value() noexcept
  { return std::numeric_limits<T>::quiet_NaN(); }

  static constexpr bool is_empty(T v) { return std::isnan(v); }
};

template<class T>
struct empty
{
  using value_type = T;
  static constexpr T empty_value() noexcept(T()) { return T(); }
  static constexpr bool is_empty(T v) { return v.empty(); }
};

}  // namespace facultative_ns

///
/// Manages an optional contained value, i.e. a value that may or may not be
/// present.
///
/// \tparam P   a policy for constructing and detecting the sentinel value
/// \tparam TAG serves only to discriminate two otherwise identical types
///
/// This is a replacement for "hidden optional values" where one sentinel
/// is stored without the spatial overhead of a `std::optional` (but giving up
/// part of the flexibility).
///
/// - the class has an interface different than that of `std::optional`. This
//    is for two reasons. First, its semantics differ, so we want to avoid the
///   confusion. Second, this library has strong focus on being unambiguous and
///   explicit at the expense of sacrifying some convenience;
/// - there is an implicit conversion from `value_type` and you should pay
///   attention to the assignment of a sentinel value (the state of having no
///   value is part of the contained value);
/// - `std::optional<T>` gives you a useful guarantee that if you initialize it
///   to a no-value state, no object of type `T` is created. This is useful for
///   run-time performance reasons and allows a two phase initialization. In
///   contrast, `facultative` upon construction, immediately constructs a
///   `value_type`.
///
/// \attention
/// Some type may not have a "spare" value to indicate the empty state. In
/// such case, `facultative` cannot help you.
///
/// \see
/// Andrzej Krzemienski's article:
/// https://akrzemi1.wordpress.com/2015/07/15/efficient-optional-values/
///
template<class P, class TAG = facultative_ns::default_tag>
class facultative_with_policy
{
  static_assert(!std::is_same<typename P::value_type, bool>::value,
                "facultative_with_policy doesn't support bool type.");

public:
  using value_type = typename P::value_type;

  constexpr facultative_with_policy()
    noexcept(std::is_nothrow_copy_constructible<value_type>::value)
    : val_(P::empty_value())
  {
  }

  constexpr facultative_with_policy(const value_type &v)
    noexcept(std::is_nothrow_copy_constructible<value_type>::value)
    : val_(v)
  {
  }

  constexpr facultative_with_policy(value_type &&v)
    noexcept(std::is_nothrow_move_constructible<value_type>::value)
    : val_(std::move(v))
  {
  }

  explicit facultative_with_policy(const std::string &s)
  {
    std::istringstream ss(s);
    if (!(ss >> val_))
      val_ = P::empty_value();
  }

  template<class U>
  constexpr facultative_with_policy &operator=(const U &v)
  {
    val_ = v;
    return *this;
  }

  void reset() noexcept(noexcept(facultative_with_policy()))
  {
    *this = facultative_with_policy();
  }

  friend void swap(facultative_with_policy &lhs, facultative_with_policy &rhs)
  {
    std::swap(lhs.val_, rhs.val_);
  }

  constexpr const value_type &operator*() const { return val_; }
  //constexpr value_type &operator*() { return val_; }
  constexpr const value_type &value() const { return **this; }
  //constexpr value_type &value() { return **this; }

  /// Returns the contained value if `*this` has a value, otherwise returns
  /// `default_value`.
  constexpr value_type value_or(const value_type &default_value) const
  { return has_value() ? **this : default_value; }

  constexpr bool has_value() const { return !P::is_empty(val_); }

private:
  value_type val_;
};  // class facultative_with_policy

template<class T, T V = std::numeric_limits<T>::max()>
using facultative = facultative_with_policy<facultative_ns::value<T, V>>;

}  // namespace vita

#endif // include guard
