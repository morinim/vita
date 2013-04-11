/**
 *
 *  \file random.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "random.h"

namespace vita
{
  // Magic!!!
  random::base_gen random::rng_(28071973u);

  ///
  /// \param[in] s the seed for the random number generator.
  ///
  /// The seed is used to initalize the random number generator. With the same
  /// seed the numbers produced will be the same every time the program is
  /// run. One common method to change this is to seed with the current time
  /// (\c std::time(0) ).
  ///
  void random::seed(unsigned s)
  {
    rng_.seed(s);
  }

  ///
  /// \param[in] base
  /// \param[in] width
  /// \param[in] n
  /// \return the index of the choosen individual.
  ///
  /// Return a random number in the range [base-width/2, base+width/2] mod n.
  /// This is like a circular protractor marked from 0 to \c n: starting from
  /// position \a base we want a random number whose distance from \a base
  /// is less than or equal to \c width/2.
  /// If \c base is greater than \c n we take a random starting position on the
  /// protractor.
  ///
  size_t random::ring(size_t base, size_t width, size_t n)
  {
    assert(width);
    assert(n > 1);

    if (base >= n)
      return random::between<size_t>(0, n);

    if (width > n)
      width = n;

    const unsigned offset(n + base - width/2);

    return (offset + random::between<size_t>(0, width)) % n;
  }
}  // Namespace vita
