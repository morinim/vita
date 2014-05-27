/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>

#include "kernel/individual/mep.h"
#include "kernel/ttable.h"
#include "kernel/src/primitive/factory.h"
#include "kernel/timer.h"

//
// Performs a speed test on the transposition table (insert-find cycle).
//
int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 50);

  vita::symbol_set sset;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFE"));

  const auto n(static_cast<unsigned>(argc > 2 ? std::atoi(argv[2]) : 10000000));

  vita::ttable cache(static_cast<unsigned>(argc > 3 ? std::atoi(argv[3]) : 16));

  std::vector<vita::individual> pool;
  for (size_t i(0); i < 1000; ++i)
    pool.emplace_back(env, sset);

  vita::timer t;
  for (unsigned i(0); i < n; ++i)
  {
    vita::fitness_t f({static_cast<vita::fitness_t::base_t>(i)});
    const vita::individual &ind(vita::random::element(pool));

    cache.insert(ind.signature(), f);

    cache.find(ind.signature(), &f);

    if (i % 1000 == 0)
      std::cout << i << '\r' << std::flush;
  }

  std::cout << static_cast<unsigned>(1000.0 * n / t.elapsed())
            << " store/read sec" << std::endl;

  return EXIT_SUCCESS;
}
