/**
 *  \file
 *  \remark This file is part of VITA.
 *  \details Building blocks run test.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
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

  problem p;

  p.env.init();
  p.env.mep.code_length = static_cast<unsigned>(argc > 1
                                                ? std::atoi(argv[1])
                                                : 5);
  const auto n(static_cast<unsigned>(argc > 2 ? std::atoi(argv[2]) : 1));

  symbol_factory factory;
  p.sset.insert(factory.make(domain_t::d_double, -200, 200));
  p.sset.insert(factory.make("FADD"));
  p.sset.insert(factory.make("FSUB"));
  p.sset.insert(factory.make("FMUL"));
  p.sset.insert(factory.make("FIFL"));
  p.sset.insert(factory.make("FIFE"));
  p.sset.insert(factory.make("FABS"));
  p.sset.insert(factory.make("FLN"));

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual with an effective size
    // greater than 4.
    i_mep base(p);
    auto base_es(base.active_symbols());
    while (base_es < 5)
    {
      base = i_mep(p);
      base_es = base.active_symbols();
    }

    std::cout << std::string(40, '-') << "\nBASE\n" << base << '\n';

    auto bl(base.blocks());
    for (const locus &l : bl)
    {
      i_mep blk(base.get_block(l));

      std::cout << "\nBLOCK at locus " << l << '\n' << blk;
      const auto val(interpreter<i_mep>(&blk).run());
      if (has_value(val))
        std::cout << "Output: " << lexical_cast<std::string>(val);
      else
        std::cout << "Empty output.";
      std::cout << '\n';

      if (blk.active_symbols() <= 20)
      {
        auto generalized (blk.generalize(2, p.sset));

        const auto &blk2(generalized.first);
        const auto &replaced(generalized.second);

        std::vector<index_t> positions(replaced.size());
        cvect categories(replaced.size());
        for (unsigned j(0); j < replaced.size(); ++j)
        {
          positions[j]  = replaced[j].index;
          categories[j] = replaced[j].category;
        }

        auto f(p.insert<adf>(blk2, categories));
        std::cout << '\n' << f->name() << '\n' << blk2;

        i_mep blk3(blk.replace({{f, positions}}));
        std::cout << '\n' << blk3;
        const auto val3(interpreter<i_mep>(&blk3).run());
        if (has_value(val3))
          std::cout << "Output: " << lexical_cast<std::string>(val3);
        else
          std::cout << "Empty output.";
        std::cout << "\n\n";

        if (val != val3)
        {
          std::cerr << "ADF EVAL ERROR.\n";
          return EXIT_FAILURE;
        }
      }
      else
        std::cout << "Skipping block at line " << l << '\n';
    }
  }
}
