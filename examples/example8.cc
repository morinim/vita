/**
 *
 *  \file example8.cc
 *  \remark This file is part of VITA.
 *  \details Building blocks run test.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "adf.h"
#include "distribution.h"
#include "environment.h"
#include "interpreter.h"
#include "primitive/factory.h"

int main(int argc, char *argv[])
{
  using namespace vita;

  environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 5;
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

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual with an effective size
    // greater than 4.
    individual base(env, true);
    unsigned base_es(base.eff_size());
    while (base_es < 5)
    {
      base = individual(env, true);
      base_es = base.eff_size();
    }

    std::cout << std::string(40, '-') << std::endl << "BASE" << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    std::list<locus> bl(base.blocks());
    for (auto i(bl.begin()); i != bl.end(); ++i)
    {
      individual blk(base.get_block(*i));

      std::cout << std::endl << "BLOCK at locus " << *i << std::endl;
      blk.list(std::cout);
      const any val((interpreter(blk))());
      if (val.empty())
        std::cout << "Incorrect output.";
      else
        std::cout << "Output: " << interpreter::to_string(val);
      std::cout << std::endl;

      if (blk.eff_size() <= 20)
      {
        std::vector<locus> loci;
        individual blk2(blk.generalize(2, &loci));

        std::vector<index_t> positions(loci.size());
        std::vector<category_t> categories(loci.size());
        for (unsigned j(0); j < loci.size(); ++j)
        {
          positions[j]  = loci[j][0];
          categories[j] = loci[j][1];
        }

        symbol_ptr f(new adf(blk2, categories, 100));
        env.insert(f);
        std::cout << std::endl << f->display() << std::endl;
        blk2.list(std::cout);

        individual blk3(blk.replace(f, positions));
        std::cout << std::endl;
        blk3.list(std::cout);
        const any val3((interpreter(blk3))());
        if (val3.empty())
          std::cout << "Incorrect output.";
        else
          std::cout << "Output: " << interpreter::to_string(val3);
        std::cout << std::endl << std::endl;

        if (val.empty() != val3.empty() ||
            (!val.empty() && !val3.empty() &&
             interpreter::to_string(val) != interpreter::to_string(val3)))
        {
          std::cout << "ADF EVAL ERROR." << std::endl;
          return EXIT_FAILURE;
        }
      }
      else
        std::cout << "Skipping block at line " << *i << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
