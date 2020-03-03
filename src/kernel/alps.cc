/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <limits>

#include "kernel/alps.h"

namespace vita::alps
{

///
/// \param[in] l a layer.
/// \param[in] age_gap see environment::age_gap data member.
/// \return the maximum allowed age for an individual in layer `l`.
///
unsigned max_age(unsigned l, unsigned age_gap)
{
  // A polynomial aging scheme.
  switch (l)
  {
  case 0:   return age_gap;
  case 1:   return age_gap + age_gap;
  default:  return l * l * age_gap;
  }

  // A linear aging scheme.
  // return age_gap * (l + 1);

  // An exponential aging scheme.
  // switch (l)
  // {
  // case 0:  return age_gap;
  // case 1:  return age_gap + age_gap;
  // default:
  // {
  //   auto k(4);
  //   for (unsigned i(2); i < layer; ++i)
  //     k *= 2;
  //   return k * age_gap;
  // }

  // Fibonacci aging scheme.
  // auto num1(age_gap), num2(age_gap);
  // while (num2 <= 2)
  // {
  //   auto num3(num2);
  //   num2 += num1;
  //   num1 = num3;
  // }
  //
  // if (l == 1)
  //   return num1 + num2 - 1;
  //
  // for (unsigned i(1); i <= l; ++i)
  // {
  //   auto num3(num2);
  //   num2 += num1 -1;
  //   num1 = num3;
  // }
  // return num2;
}

}  // namespace vita::alps
