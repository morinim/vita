/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_UTILITY_H)
#define      VITA_UTILITY_H

#include <algorithm>

#include "kernel/vita.h"

namespace vita
{
  ///
  /// \tparam T a C++ type.
  /// \return the maximum value of type \a T.
  ///
  template<class T> constexpr T type_max(T)
  {
    return std::numeric_limits<T>::max();
  }

  ///
  /// An implementation of make_unique() as proposed by Herb Sutter in
  /// GotW #102.
  ///
  template<typename T, typename ...Args>
  std::unique_ptr<T> make_unique(Args&& ...args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  ///
  /// \return an std::array filled with value \a v.
  ///
  /// This function is used to initialize array in member initialization list
  /// of a constructor (so to be compliant with Effective C++).
  /// To be efficient the compiler need to perform the RVO optimization / copy
  /// elision.
  ///
  /// \see
  /// <http://stackoverflow.com/questions/21993780/fill-stdarray-in-the-member-initialization-list>
  ///
  template<class T, unsigned N> std::array<T, N> make_array(T v)
  {
    std::array<T, N> temp;
    temp.fill(v);
    return temp;
  }

  ///
  /// \return \c true if v is less than epsilon-tolerance.
  ///
  /// \note
  /// \a epsilon is the smallest T-value that can be added to \c 1.0 without
  /// getting \c 1.0 back. Note that this is a much larger value than
  /// \c DBL_MIN.
  ///
  template<class T> inline bool issmall(T v)
  {
    static constexpr auto epsilon(std::numeric_limits<T>::epsilon());

    return std::abs(v) < 2.0 * epsilon;
  }

  ///
  /// \param[in] container a STL container.
  /// \param[in] pred a unary predicate.
  ///
  /// A shortcut for the well known C++ erase-remove idiom.
  ///
  template<class T, class Pred> void erase_if(T &container, Pred pred)
  {
    container.erase(std::remove_if(container.begin(),
                                   container.end(),
                                   pred),
                    container.end());
  }
}  // namespace vita

#endif  // Include guard
