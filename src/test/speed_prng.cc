/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <random>

#include "utility/timer.h"
#include "utility/xoshiro256ss.h"

int main()
{
  volatile std::uint64_t out;
  unsigned sup(-1);

  // -------------------------------------------------------------------------

  std::mt19937 e1;

  vita::timer t;
  for (decltype(sup) i(0); i < sup; ++i)
    out = e1();

  std::cout << "MT19937       - Elapsed: " << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------

  std::mt19937_64 e2;

  t.restart();
  for (decltype(sup) i(0); i < sup; ++i)
    out = e2();

  std::cout << "MT19937_64    - Elapsed: " << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------
  vigna::xoroshiro128p e3;

  t.restart();
  for (decltype(sup) i(0); i < sup; ++i)
    out = e3();

  std::cout << "XOROSHIRO128+ - Elapsed: " << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------
  vigna::xoshiro256ss e4;

  t.restart();
  for (decltype(sup) i(0); i < sup; ++i)
    out = e4();

  std::cout << "XOSHIRO256**  - Elapsed: " << t.elapsed().count() << "ms\n";

  return !out;  // just to stop some warnings
}
