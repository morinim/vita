/**
 *  \file
 *  \remark This file is part of VITA.
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

#include "kernel/environment.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"
#include "kernel/src/primitive/factory.h"

int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.code_length = argc > 1 ? atoi(argv[1]) : 14;

  vita::symbol_set sset;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  sset.insert(factory.make(vita::domain_t::d_double, -200, 200));
  sset.insert(factory.make("FADD"));
  sset.insert(factory.make("FIFE"));
  sset.insert(factory.make("FIFL"));
  sset.insert(factory.make("FIFZ"));
  sset.insert(factory.make("FMOD"));
  sset.insert(factory.make("FMUL"));
  sset.insert(factory.make("FSUB"));
  sset.insert(factory.make("FLENGTH", {1, 0}));
  sset.insert(factory.make("apple", {1}));
  sset.insert(factory.make("grapefruit", {1}));
  sset.insert(factory.make("orange", {1}));

  vita::individual ind(env, sset);

  ind.dump(std::cout);
  std::cout << std::endl;

  const vita::any val(vita::interpreter<vita::individual>(ind).run());
  if (val.empty())
    std::cout << "Incorrect program." << std::endl;
  else
    std::cout << "Output: " << vita::to<std::string>(val) << std::endl;

  return EXIT_SUCCESS;
}
