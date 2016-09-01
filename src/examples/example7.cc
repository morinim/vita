/**
 *  \file
 *  \remark This file is part of VITA.
 *  \details Building blocks infrastructure test.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <iostream>
#include <fstream>

#include "kernel/vita.h"

int main(int argc, char *argv[])
{
  using namespace vita;

  vita::symbol_set sset;
  environment env(&sset, true);

  env.code_length = static_cast<unsigned>(argc > 1 ? std::atoi(argv[1]) : 100);
  const auto n(static_cast<unsigned>(argc > 2 ? std::atoi(argv[2]) : 1));

  symbol_factory factory;
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
    i_mep base(env);
    auto base_es(base.eff_size());
    while (base_es < 5)
    {
      base = i_mep(env);
      base_es = base.eff_size();
    }

    individuals.add(base_es);

    std::cout << std::string(40, '-') << '\n' << base << '\n';

    auto bl(base.blocks());
    for (auto i(bl.begin()); i != bl.end(); ++i)
    {
      i_mep ib(base.get_block(*i));

      auto generalized(ib.generalize(2, *env.sset));

      std::cout << '\n' << ib
                << "GENERALIZED\n" << generalized.first
                << "\nArguments: [";
      for (const auto &l : generalized.second)
        std::cout << ' ' << l;
      std::cout << " ]\n";

      blocks_len.add(ib.eff_size());
      arguments.add(static_cast<double>(generalized.second.size()));
    }
  }

  std::cout << std::string(40, '-')
            << "\nIndividuals effective lengths.\nMin: " << individuals.min()
            << "  Mean: " << individuals.mean()
            << "  StdDev: " << individuals.standard_deviation()
            << "  Max: " << individuals.max()
            << "\nBlocks effective lengths.\nMin: " << blocks_len.min()
            << "  Mean: " << blocks_len.mean()
            << "  StdDev: " << std::sqrt(blocks_len.variance())
            << "  Max: " << blocks_len.max()
            << "\nNumber of arguments.\nMin: " << arguments.min()
            << "  Mean: " << arguments.mean()
            << "  StdDev: " << arguments.standard_deviation()
            << "  Max: " << arguments.max() << std::endl;
}
