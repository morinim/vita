/**
 *
 *  \file example4.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>

#include "environment.h"
#include "individual.h"
#include "ttable.h"
#include "primitive/factory.h"

//
// Performs a speed test on the transposition table (insert-find cycle).
//
int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 50;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make(vita::d_double, -200, 200));
  env.insert(factory.make("FADD"));
  env.insert(factory.make("FSUB"));
  env.insert(factory.make("FMUL"));
  env.insert(factory.make("FIFL"));
  env.insert(factory.make("FIFE"));

  const unsigned n(argc > 2 ? atoi(argv[2]) : 4000000);

  vita::ttable cache(argc > 3 ? atoi(argv[3]) : 16);

  const clock_t start(clock());
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, true);

    vita::fitness_t f({i});
    cache.insert(i1, f);

    cache.find(i1, &f);

    if (i % 1000 == 0)
      std::cout << i << '\r' << std::flush;
  }
  const clock_t end(clock());

  std::cout << static_cast<double>(n) * CLOCKS_PER_SEC / (end - start)
            << " store/read sec" << std::endl;

  return EXIT_SUCCESS;
}
