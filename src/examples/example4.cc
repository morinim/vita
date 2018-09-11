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

#include <iostream>

#include "kernel/vita.h"

//
// Performs a speed test on the transposition table (insert-find cycle).
//
int main(int argc, char *argv[])
{
  using namespace vita;
  problem p;

  p.env.init();
  p.env.mep.code_length = static_cast<unsigned>(argc > 1
                                                ? std::atoi(argv[1]) : 50);

  symbol_factory factory;
  p.sset.insert(factory.make(domain_t::d_double, -200, 200));
  p.sset.insert(factory.make("FADD"));
  p.sset.insert(factory.make("FSUB"));
  p.sset.insert(factory.make("FMUL"));
  p.sset.insert(factory.make("FIFL"));
  p.sset.insert(factory.make("FIFE"));

  const auto n(static_cast<unsigned>(argc > 2 ? std::atoi(argv[2]) : 10000000));

  cache cache(static_cast<unsigned>(argc > 3 ? std::atoi(argv[3]) : 16));

  std::vector<i_mep> pool;
  for (size_t i(0); i < 1000; ++i)
    pool.emplace_back(p);

  timer t;
  for (unsigned i(0); i < n; ++i)
  {
    fitness_t f{static_cast<fitness_t::value_type>(i)};
    const i_mep &ind(random::element(pool));

    cache.insert(ind.signature(), f);
    cache.find(ind.signature());

    if (i % 1000 == 0)
      std::cout << i << '\r' << std::flush;
  }

  std::cout << 1000.0l * n * t.elapsed().count() << " store/read sec\n";
}
