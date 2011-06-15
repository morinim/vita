/**
 *
 *  \file random.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kernel/random.h"

namespace vita
{
  random::base_gen random::rng_(28071973u);

  ///
  /// \param[in] s the seed for the random number generator.
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
  unsigned random::ring(unsigned base, unsigned width, unsigned n)
  {
    assert(width && n>1);

    if (base >= n)
      return random::between<unsigned>(0, n);

    if (width > n)
      width = n;

    const unsigned offset(n+base - width/2);

    return (offset + random::between<unsigned>(0, width)) % n;
  }
}  // Namespace vita
