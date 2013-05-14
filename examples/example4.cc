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

#include "individual.h"
#include "ttable.h"
#include "primitive/factory.h"
#include "timer.h"

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

  const unsigned n(argc > 2 ? atoi(argv[2]) : 10000000);

  vita::ttable cache(argc > 3 ? atoi(argv[3]) : 16);

  std::vector<vita::individual> pool;
  for (size_t i(0); i < 1000; ++i)
    pool.emplace_back(env, true);

  vita::timer t;
  for (unsigned i(0); i < n; ++i)
  {
    vita::fitness_t f({static_cast<vita::fitness_t::base_t>(i)});
    const vita::individual &ind(vita::random::element(pool));

    cache.insert(ind, f);

    cache.find(ind, &f);

    if (i % 1000 == 0)
      std::cout << i << '\r' << std::flush;
  }

  std::cout << static_cast<unsigned>(1000.0 * n / t.elapsed())
            << " store/read sec" << std::endl;

  return EXIT_SUCCESS;
}
