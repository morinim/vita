/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_RANGE_H)
#define      VITA_RANGE_H

namespace vita
{

///
/// Right-open interval.
///
/// `range_t{m, u}` specifies the half-open (left-closed, right-open) interval
/// `[m, u[`.
///
template<class T> using range_t = std::pair<T, T>;

template<class T1, class T2>
constexpr std::pair<T1, T2> range(T1 &&m, T2 &&u)
{
  return std::pair<T1, T2>(std::forward<T1>(m), std::forward<T2>(u));
}

}  // namespace vita

#endif
