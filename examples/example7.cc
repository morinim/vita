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
#include "kernel/primitive/sr_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 100;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

  vita::symbol_ptr s1(new vita::sr::number(-200, 200));
  vita::symbol_ptr s2(new vita::sr::add());
  vita::symbol_ptr s3(new vita::sr::sub());
  vita::symbol_ptr s4(new vita::sr::mul());
  vita::symbol_ptr s5(new vita::sr::ifl());
  vita::symbol_ptr s6(new vita::sr::ife());
  vita::symbol_ptr s7(new vita::sr::abs());
  vita::symbol_ptr s8(new vita::sr::ln());
  env.insert(s1);
  env.insert(s2);
  env.insert(s3);
  env.insert(s4);
  env.insert(s5);
  env.insert(s6);
  env.insert(s7);
  env.insert(s8);

  vita::distribution<double> individuals, blocks_len, blocks_n, arguments;

  for (unsigned k(0); k < n; ++k)
  {
    vita::individual base(env, true);
    unsigned base_es(base.eff_size());
    while (base_es < 5)
    {
      base = vita::individual(env, true);
      base_es = base.eff_size();
    }

    individuals.add(base_es);

    std::cout << std::string(40, '-') << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    std::list<unsigned> bl(base.blocks());
    for (std::list<unsigned>::const_iterator i(bl.begin()); i != bl.end(); ++i)
    {
      vita::individual ib(base.get_block(*i));
      unsigned first_terminal;
      vita::individual opt(ib.optimize(&first_terminal));

      if (first_terminal)
      {
        std::vector<unsigned> positions;
        std::vector<vita::symbol_t> types;
        opt.generalize(2, &positions, &types);

        std::cout << std::endl;
        ib.list(std::cout);

        std::cout << "OPTIMIZED" << std::endl;
        opt.list(std::cout);

        const unsigned arg_n(positions.size());
        std::cout << std::endl << "Arguments:";
        for (unsigned j(0); j < arg_n; ++j)
          std::cout << " (pos=" << positions[j] << ",type=" << types[j]
                    << ")";
        std::cout << std::endl;

        blocks_len.add(ib.eff_size());
        arguments.add(arg_n);
      }
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
