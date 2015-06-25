/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ALPS_H)
#define      VITA_ALPS_H

#include "kernel/population.h"

namespace vita
{
///
/// alps namespace contains some support functions for the ALPS algorithm
///
namespace alps
{
unsigned max_age(unsigned, unsigned);
unsigned max_age(unsigned, unsigned, unsigned);

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
  return p[c].age() > max_age(c.layer, p.layers(), p.env().alps.age_gap);
}

}  // namespace alps
}  // namespace vita

#endif  // Include guard
