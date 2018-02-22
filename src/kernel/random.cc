/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
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
/// The shared random engine generator.
///
/// Every thread has its own generator.
/// The numbers produced will be the same every time the program is run.
///
thread_local engine_t engine;

///
/// Initalizes the random number generator.
///
/// \param[in] s the seed for the random number generator
///
/// The seed is used to initalize the random number generator. With the same
/// seed the numbers produced will be the same every time the program is run.
///
/// \note
/// A common method to seed a PRNG is using the current time (`std::time(0)`).
/// It works... but the preferred way in Vita is the `randomize` method (which
/// is based on `std::random_device`).
///
void seed(unsigned s)
{
  engine.seed(s);
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
/// Returns a random number in a modular arithmetic system.
///
/// \param[in] base  a base number
/// \param[in] width maximum distance from the `base` number
/// \param[in] n     modulus
/// \return          a random number in the
///                  `[base - width/2, base + width/2] mod n` interval
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
