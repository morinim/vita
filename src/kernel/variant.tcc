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
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_VARIANT_TCC)
#define      VITA_VARIANT_TCC

// Implementation details, user shouldn't import this.
namespace detail { namespace variant_
{
///////////////////////////////////////////////////////////////////////////////
template<int N, class T, class... Ts>
struct storage_ops<N, T &, Ts...>
{
  static void del(int, void *) {}

  template<class visitor>
  static typename visitor::result_type apply(int, void *, visitor &) {}
};

template<int N, class T, class... Ts>
struct storage_ops<N, T, Ts...>
{
  static void del(int n, void *data)
  {
    if (n == N)
      reinterpret_cast<T *>(data)->~T();
    else
      storage_ops<N + 1, Ts...>::del(n, data);
  }

  template<class visitor>
  static typename visitor::result_type apply(int n, void *data, visitor &v)
  {
    if (n == N)
      return v(*reinterpret_cast<T *>(data));
    else
      return storage_ops<N + 1, Ts...>::apply(n, data, v);
  }
};

template<int N>
struct storage_ops<N>
{
  static void del(int, void *)
  {
    throw std::runtime_error("Internal error: variant tag is invalid");
  }

  template<class visitor>
  static typename visitor::result_type apply(int, void *, visitor &)
  {
    throw std::runtime_error("Internal error: variant tag is invalid");
  }
};

///////////////////////////////////////////////////////////////////////////////
template<class X>
struct position<X>
{
  static constexpr int pos = -1;
};

template<class X, class... Ts>
struct position<X, X, Ts...>
{
  static constexpr int pos = 0;
};

template<class X, class T, class... Ts>
struct position<X, T, Ts...>
{
  static constexpr int pos = position<X, Ts...>::pos != -1
    ? position<X, Ts...>::pos + 1 : -1;
};

///////////////////////////////////////////////////////////////////////////////
template<class T, class... Ts>
struct type_info<T, Ts...>
{
  static constexpr bool no_reference_types =
    !std::is_reference<T>::value && type_info<Ts...>::no_reference_types;

  static constexpr bool no_duplicates =
    position<T, Ts...>::pos == -1 && type_info<Ts...>::no_duplicates;

  static constexpr std::size_t size =
    type_info<Ts...>::size > sizeof(T) ? type_info<Ts...>::size : sizeof(T);

  static constexpr std::size_t alignment =
    type_info<Ts...>::alignment > alignof(T) ? type_info<Ts...>::alignment
                                             : alignof(T);
};

template<>
struct type_info<>
{
  static constexpr bool no_reference_types = true;
  static constexpr bool no_duplicates = true;
  static constexpr std::size_t size = 0;
  static constexpr std::size_t alignment = 0;
};
}}  // namespace detail::variant_

template<class... Ts>
template<class X>
variant<Ts...>::variant(const X &v)
{
  static_assert(detail::variant_::position<X, Ts...>::pos != -1,
                "Type not in variant");
  init(v);
}

template<class... Ts>
variant<Ts...>::~variant()
{
  detail::variant_::storage_ops<0, Ts...>::del(tag_, storage_);
}

template<class... Ts>
template<class X>
void variant<Ts...>::operator=(const X &v)
{
  static_assert(detail::variant_::position<X, Ts...>::pos != -1,
                "Type not in variant");
  this->~variant();
  init(v);
}

///
/// \return a reference to the content of the variant
///
/// Run-time checked explicit value retrival of the content of the variant.
///
template<class... Ts>
template<class X>
X &variant<Ts...>::get()
{
  static_assert(detail::variant_::position<X, Ts...>::pos != -1,
                "Type not in variant");

  // Dereferencing the pointer is well defined:
  // <http://stackoverflow.com/q/28381338/3235496>
  if (tag_ == detail::variant_::position<X, Ts...>::pos)
    return *reinterpret_cast<X *>(storage_);

  throw std::runtime_error("variant doesn't contain requested type (allowed " +
                           std::to_string(tag_) + ")");
}

///
/// \return a const reference to the content of the variant
///
/// Run-time checked explicit value retrival of the content of the variant.
///
template<class... Ts>
template<class X>
const X &variant<Ts...>::get() const
{
  static_assert(detail::variant_::position<X, Ts...>::pos != -1,
                "Type not in variant");

  if (tag_ == detail::variant_::position<X, Ts...>::pos)
    return *reinterpret_cast<const X *>(storage_);

  throw std::runtime_error("variant doesn't contain requested type (allowed" +
                           std::to_string(tag_) + ")");
}

template<class... Ts>
template<class visitor>
typename visitor::result_type variant<Ts...>::visit(visitor &v)
{
  return detail::variant_::storage_ops<0, Ts...>::apply(tag_, storage_, v);
}

template<class... Ts>
int variant<Ts...>::which() const
{
  return tag_;
}

#endif  // Include guard
