/**
 *  \file
 *  \remark This file is part of VITA.
 *  \details Building blocks run test.
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
#include "kernel/interpreter.h"
#include "kernel/src/primitive/factory.h"

int main(int argc, char *argv[])
{
  using namespace vita;

  environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 5;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

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

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual with an effective size
    // greater than 4.
    individual base(env, sset);
    auto base_es(base.eff_size());
    while (base_es < 5)
    {
      base = individual(env, sset);
      base_es = base.eff_size();
    }

    std::cout << std::string(40, '-') << std::endl << "BASE" << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    auto bl(base.blocks());
    for (const locus &l : bl)
    {
      individual blk(base.get_block(l));

      std::cout << std::endl << "BLOCK at locus " << l << std::endl;
      blk.list(std::cout);
      const any val(interpreter<individual>(blk).run());
      if (val.empty())
        std::cout << "Empty output.";
      else
        std::cout << "Output: " << to<std::string>(val);
      std::cout << std::endl;

      if (blk.eff_size() <= 20)
      {
        auto generalized (blk.generalize(2));

        const auto &blk2(generalized.first);
        const auto &replaced(generalized.second);

        std::vector<index_t> positions(replaced.size());
        std::vector<category_t> categories(replaced.size());
        for (unsigned j(0); j < replaced.size(); ++j)
        {
          positions[j]  = replaced[j].index;
          categories[j] = replaced[j].category;
        }

        symbol *const f(sset.insert(vita::make_unique<adf>(blk2, categories,
                                                           100)));
        std::cout << std::endl << f->display() << std::endl;
        blk2.list(std::cout);

        individual blk3(blk.replace({{f, positions}}));
        std::cout << std::endl;
        blk3.list(std::cout);
        const any val3(interpreter<individual>(blk3).run());
        if (val3.empty())
          std::cout << "Empty output.";
        else
          std::cout << "Output: " << to<std::string>(val3);
        std::cout << std::endl << std::endl;

        if (val.empty() != val3.empty() ||
            (!val.empty() && !val3.empty() &&
             to<std::string>(val) != to<std::string>(val3)))
        {
          std::cerr << "ADF EVAL ERROR." << std::endl;
          return EXIT_FAILURE;
        }
      }
      else
        std::cout << "Skipping block at line " << l << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
