/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://bitbucket.org/morinim/vita/wiki/path_finding_tutorial
 */

#include <iostream>
#include <string>

#include "kernel/vita.h"

const std::string GENESET =
  " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!";

// Encodes a letter of the string to be guessed.
class letter : public vita::ga::integer
{
public:
  letter() : vita::ga::integer({0, GENESET.length()})  {}

  std::string display(terminal::param_t v, format) const override
  {
    return std::string(1, GENESET[static_cast<unsigned>(v)]);
  }
};

int main()
{
  using namespace vita;

  problem prob;
  prob.env.individuals = 300;
  prob.env.generations = 100;


  const std::string target = "Hello World";
  prob.chromosome<letter>(target.length());

  // The fitness function.
  auto f = [&target](const i_ga &x) -> fitness_t
  {
    double found(0);

    for (std::size_t i = 0; i < target.length(); ++i)
      if (target[i] == GENESET[x[i].as<int>()])
        ++found;

    return {found};
  };

  ga_search<i_ga, std_es, decltype(f)> search(prob, f);
  auto result = search.run();

  std::cout << "\nBest result: ";
  for (auto gene :  result.best.solution)
    std::cout << GENESET[gene.as<int>()];

  std::cout << " (fitness " << result.best.score.fitness << ")\n";
}
