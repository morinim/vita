/**
 *
 *  \file example7.cc
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
 *
 *  Building blocks infrastructure test.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "kernel/adf.h"
#include "kernel/distribution.h"
#include "kernel/environment.h"
#include "kernel/primitive/factory.h"

int main(int argc, char *argv[])
{
  using namespace vita;
  environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 100;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

  symbol_factory &factory(symbol_factory::instance());
  env.insert(factory.make(d_double, -200, 200));
  env.insert(factory.make("FADD"));
  env.insert(factory.make("FSUB"));
  env.insert(factory.make("FMUL"));
  env.insert(factory.make("FIFL"));
  env.insert(factory.make("FIFE"));
  env.insert(factory.make("FABS"));
  env.insert(factory.make("FLN"));

  distribution<double> individuals, blocks_len, blocks_n, arguments;

  for (unsigned k(0); k < n; ++k)
  {
    individual base(env, true);
    unsigned base_es(base.eff_size());
    while (base_es < 5)
    {
      base = individual(env, true);
      base_es = base.eff_size();
    }

    individuals.add(base_es);

    std::cout << std::string(40, '-') << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    std::list<locus> bl(base.blocks());
    for (auto i(bl.begin()); i != bl.end(); ++i)
    {
      individual ib(base.get_block(*i));

      std::vector<locus> arg_loc;
      individual generalized(ib.generalize(2, &arg_loc));

      std::cout << std::endl;
      ib.list(std::cout);

      std::cout << "GENERALIZED" << std::endl;
      generalized.list(std::cout);

      std::cout << std::endl << "Arguments: [";
      for (unsigned j(0); j < arg_loc.size(); ++j)
        std::cout << ' ' << arg_loc[j];
      std::cout << " ]" << std::endl;

      blocks_len.add(ib.eff_size());
      arguments.add(arg_loc.size());
    }
  }

  std::cout << std::string(40, '-') << std::endl
            << "Individuals effective lengths." << std::endl
            << "Min: " << individuals.min << "  Mean: " << individuals.mean
            << "  StdDev: " << std::sqrt(individuals.variance)
            << "  Max: " << individuals.max << std::endl
            << "Blocks effective lengths." << std::endl
            << "Min: " << blocks_len.min << "  Mean: " << blocks_len.mean
            << "  StdDev: " << std::sqrt(blocks_len.variance)
            << "  Max: " << blocks_len.max << std::endl
            << "Number of arguments." << std::endl
            << "Min: " << arguments.min << "  Mean: " << arguments.mean
            << "  StdDev: " << std::sqrt(arguments.variance)
            << "  Max: " << arguments.max << std::endl;

  return EXIT_SUCCESS;
}
