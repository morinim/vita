/**
 *  \file
 *  \remark This file is part of VITA.
 *  \details Building blocks infrastructure test.
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
#include <fstream>

#include "kernel/adf.h"
#include "kernel/distribution.h"
#include "kernel/environment.h"
#include "kernel/src/primitive/factory.h"

int main(int argc, char *argv[])
{
  using namespace vita;
  environment env(true);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 100);
  const auto n(static_cast<unsigned>(argc > 2 ? std::atoi(argv[2]) : 1));

  vita::symbol_set sset;

  symbol_factory &factory(symbol_factory::instance());
  sset.insert(factory.make(domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFE"));
  sset.insert(factory.make("FABS"));
  sset.insert(factory.make("FLN"));

  distribution<double> individuals, blocks_len, blocks_n, arguments;

  for (unsigned k(0); k < n; ++k)
  {
    i_mep base(env, sset);
    auto base_es(base.eff_size());
    while (base_es < 5)
    {
      base = i_mep(env, sset);
      base_es = base.eff_size();
    }

    individuals.add(base_es);

    std::cout << std::string(40, '-') << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    auto bl(base.blocks());
    for (auto i(bl.begin()); i != bl.end(); ++i)
    {
      i_mep ib(base.get_block(*i));

      auto generalized(ib.generalize(2));

      std::cout << std::endl;
      ib.list(std::cout);

      std::cout << "GENERALIZED" << std::endl;
      generalized.first.list(std::cout);

      std::cout << std::endl << "Arguments: [";
      for (const auto &l : generalized.second)
        std::cout << ' ' << l;
      std::cout << " ]" << std::endl;

      blocks_len.add(ib.eff_size());
      arguments.add(static_cast<double>(generalized.second.size()));
    }
  }

  std::cout << std::string(40, '-') << std::endl
            << "Individuals effective lengths." << std::endl
            << "Min: " << individuals.min << "  Mean: " << individuals.mean()
            << "  StdDev: " << std::sqrt(individuals.variance())
            << "  Max: " << individuals.max << std::endl
            << "Blocks effective lengths." << std::endl
            << "Min: " << blocks_len.min << "  Mean: " << blocks_len.mean()
            << "  StdDev: " << std::sqrt(blocks_len.variance())
            << "  Max: " << blocks_len.max << std::endl
            << "Number of arguments." << std::endl
            << "Min: " << arguments.min << "  Mean: " << arguments.mean()
            << "  StdDev: " << std::sqrt(arguments.variance())
            << "  Max: " << arguments.max << std::endl;

  return EXIT_SUCCESS;
}
