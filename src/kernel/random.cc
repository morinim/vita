/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/random.h"

namespace vita
{
namespace random
{
///
/// \param[in] s the seed for the random number generator.
///
/// The seed is used to initalize the random number generator. With the same
/// seed the numbers produced will be the same every time the program is
/// run.
///
/// \note
/// One common method to change this is to seed with the current time
/// (\c std::time(0)) but the preferred way in Vita is the random::randomize
/// method (based on \c std::random_device).
///
void seed(unsigned s)
{
  engine().seed(s);
}

///
/// Sets the shared engine to an unpredictable state.
///
void randomize()
{
  std::random_device rd;
  seed(rd());
}

///
/// \param[in] base
/// \param[in] width
/// \param[in] n
/// \return the index of the choosen individual.
///
/// Return a random number in the interval
/// `[base - width/2, base + width/2] mod n`.
///
unsigned ring(unsigned base, unsigned width, unsigned n)
{
  Expects(base < n);
  Expects(width);
  Expects(n > 1);

  if (width >= n)
    return random::between(0u, n);

  const auto min_val(base + n - width / 2);

  return (min_val + random::between(0u, width)) % n;
}

}  // namespace random
}  // namespace vita
