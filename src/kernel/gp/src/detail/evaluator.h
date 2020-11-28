/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_EVALUATOR_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_DETAIL_SRC_EVALUATOR_H)
#define      VITA_DETAIL_SRC_EVALUATOR_H

namespace vita::detail
{

///
/// A trait to check if a container is iterable (has `begin`/`end`).
///
template<class T, class = void> struct is_iterable : std::false_type {};

template<class T>
struct is_iterable<T, std::void_t<decltype(std::declval<T>().begin()),
                                  decltype(std::declval<T>().end())>>
  : std::true_type {};

template<class T>
constexpr bool is_iterable_v = is_iterable<T>::value;

///
/// A trait to check if a container has the `classes` method.
///
template<class T, class = void> struct has_classes : std::false_type {};

template<class T>
struct has_classes<T, std::void_t<decltype(std::declval<T>().classes())>>
  : std::true_type {};

template<class T>
constexpr bool has_classes_v = has_classes<T>::value;

template<class DAT> std::size_t classes(const DAT &d)
{
  if constexpr (has_classes_v<DAT>)
    return d.classes();

  return 0;
}

///
/// A trait to check if an example has an incrementable `difficulty` field.
///
template<class T, class = void> struct has_difficulty : std::false_type {};

template<class T>
struct has_difficulty<T, std::void_t<decltype(++std::declval<T>().begin()->difficulty)>>
  : std::true_type {};

template<class T>
constexpr bool has_difficulty_v = has_difficulty<T>::value;

///
/// A trait to check if `ERRF` can be applied to elements of `DAT` returning a
/// `double`.
///
template<class ERRF, class DAT>
constexpr bool is_error_functor_v =
  std::is_invocable_r_v<double, ERRF, decltype(*std::declval<DAT>().begin())>;

}  // namespace vita::detail

#endif  // include guard
