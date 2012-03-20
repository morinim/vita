/**
 *
 *  \file example4.cc
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

#include <cstdlib>
#include <iostream>

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/ttable.h"
#include "kernel/primitive/factory.h"

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

  const unsigned n(argc > 2 ? atoi(argv[2]) : 5000);

  vita::ttable cache(argc > 3 ? atoi(argv[3]) : 16);

  const clock_t start(clock());
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env, true);

    vita::score_t s(i, 0.0);
    cache.insert(i1, s);

    cache.find(i1, &s);

    if (i % 1000 == 0)
      std::cout << i << '\r' << std::flush;
  }
  const clock_t end(clock());

  std::cout << static_cast<double>(n) * CLOCKS_PER_SEC / (end - start)
            << " store/read sec" << std::endl;

  return EXIT_SUCCESS;
}
