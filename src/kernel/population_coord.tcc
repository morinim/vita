/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_POPULATION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_POPULATION_COORD_TCC)
#define      VITA_POPULATION_COORD_TCC

///
/// Holds the coordinates of an individual in a population.
///
template<class T>
struct population<T>::coord
{
  unsigned layer;
  unsigned index;

  bool operator==(coord rhs) const
  { return layer == rhs.layer && index == rhs.index; }

  bool operator!=(coord rhs) const
  { return !(*this == rhs); }

  bool operator<(coord rhs) const
  {
    return layer < rhs.layer
           || (layer == rhs.layer && index < rhs.index);
  }
};  // struct coord

#endif  // include guard
