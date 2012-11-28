/**
 *
 *  \file example5.cc
 *  \remark This file is part of VITA.
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

#include "environment.h"
#include "individual.h"
#include "interpreter.h"
#include "primitive/factory.h"

int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 14;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(factory.make(vita::d_double, -200, 200));
  env.insert(factory.make("FADD"));
  env.insert(factory.make("FIFE"));
  env.insert(factory.make("FIFL"));
  env.insert(factory.make("FIFZ"));
  env.insert(factory.make("FMOD"));
  env.insert(factory.make("FMUL"));
  env.insert(factory.make("FSUB"));
  env.insert(factory.make("FLENGTH", {1, 0}));
  env.insert(factory.make("apple", {1}));
  env.insert(factory.make("grapefruit", {1}));
  env.insert(factory.make("orange", {1}));

  vita::individual ind(env, true);

  ind.dump(std::cout);
  std::cout << std::endl;

  vita::interpreter agent(ind);
  const vita::any val(agent());
  if (val.empty())
    std::cout << "Incorrect program." << std::endl;
  else
    std::cout << "Output: " << vita::interpreter::to_string(val) << std::endl;

  return EXIT_SUCCESS;
}
