/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017, 2017, 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ALPS_H)
#define      VITA_ALPS_H

#include "kernel/population.h"

namespace vita::alps
{

unsigned max_age(unsigned, unsigned);

///
/// \param[in] p the population.
/// \param[in] l a layer of the population.
/// \return the maximum allowed age for an individual in layer `l`. For
///         individuals in the last layer there isn't a age limit.
///
template<class T>
unsigned allowed_age(const population<T> &pop, unsigned l)
{
  const auto layers(pop.layers());
  Expects(l < layers);

  if (l + 1 == layers)
    return std::numeric_limits<unsigned>::max();

  return max_age(l, pop.get_problem().env.alps.age_gap);
}

///
/// \param[in] p the population.
/// \param[in] c the coordinates of an individual.
/// \return `true` if the individual at coordinates `c` is too old for his
///         layer.
///
/// This is just a convenience method to save some keystroke.
///
template<class T> bool aged(const population<T> &p,
                            typename population<T>::coord c)
{
  return p[c].age() > allowed_age(p, c.layer);
}

}  // namespace vita::alps

#endif  // include guard
