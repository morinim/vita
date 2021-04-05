/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <random>

#include "kernel/vita.h"

vita::problem make_problem(std::size_t prg_size)
{
  vita::problem prob;
  vita::symbol_factory factory;

  prob.sset.insert(factory.make("0.0"));
  prob.sset.insert(factory.make("1.0"));
  prob.sset.insert(factory.make("2.0"));
  prob.sset.insert(factory.make("3.0"));
  prob.sset.insert(factory.make("123.0"));
  prob.sset.insert(factory.make("-123.0"));
  prob.sset.insert(factory.make("321.0"));
  prob.sset.insert(factory.make("FABS"));
  prob.sset.insert(factory.make("FADD"));
  prob.sset.insert(factory.make("FAQ"));
  prob.sset.insert(factory.make("FCOS"));
  prob.sset.insert(factory.make("FDIV"));
  prob.sset.insert(factory.make("FIDIV"));
  prob.sset.insert(factory.make("FIFE"));
  prob.sset.insert(factory.make("FIFZ"));
  prob.sset.insert(factory.make("FLN"));
  prob.sset.insert(factory.make("FMAX"));
  prob.sset.insert(factory.make("FMUL"));
  prob.sset.insert(factory.make("FSIGMOID"));
  prob.sset.insert(factory.make("FSIN"));
  prob.sset.insert(factory.make("FSQRT"));
  prob.sset.insert(factory.make("FSUB"));

  prob.env.init().mep.code_length = prg_size;

  return prob;
}

std::uint64_t speed_random_locus()
{
  using namespace vita;

  const unsigned N(1500);

  auto prob(make_problem(100));
  std::vector<i_mep> prg;

  std::generate_n(std::back_inserter(prg), N,
                  [&prob]{ return i_mep(prob); });

  std::uint64_t dummy(0);

  unsigned padding1(30), padding2(8);

  // -------------------------------------------------------------------------
  vita::timer t;

  for (unsigned i(0); i < N; ++i)
    for (unsigned j(0); j < N*10; ++j)
    {
      const auto l(random_locus(prg[i]));
      dummy += l.index + l.category;
    }

  std::cout << std::left << std::setw(padding1) << std::setfill('.')
            << "vita::random_locus";
  std::cout << std::right << std::setw(padding2) << std::setfill('.')
            << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------
  t.restart();

  for (unsigned i(0); i < N; ++i)
    for (unsigned j(0); j < N*10; ++j)
    {
      const auto delta(random::sup(prg[i].active_symbols()));
      const auto l(std::next(prg[i].begin(), delta).locus());
      dummy += l.index + l.category;
    }

  std::cout << std::left << std::setw(padding1) << std::setfill('.')
            << "std::next";
  std::cout << std::right << std::setw(padding2) << std::setfill('.')
            << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------
  t.restart();

  for (unsigned i(0); i < N; ++i)
    for (unsigned j(0); j < N*10; ++j)
    {
      locus l;
      std::size_t k(0);

      for (auto iter(prg[i].begin()); iter != prg[i].end(); ++iter)
        if (random::sup(++k) == 0)
          l = iter.locus();

      dummy += l.index + l.category;
    }

  std::cout << std::left << std::setw(padding1) << std::setfill('.')
            << "reservoir - iterator based";
  std::cout << std::right << std::setw(padding2) << std::setfill('.')
            << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------
  t.restart();

  for (unsigned i(0); i < N; ++i)
    for (unsigned j(0); j < N*10; ++j)
    {
      auto l(prg[i].best());
      const auto args0(prg[i][l].arguments());
      std::set left(args0.begin(), args0.end());
      std::size_t seen(1);

      for (auto iter(left.cbegin()); iter != left.end(); ++iter)
      {
        if (random::sup(++seen) == 0)
          l = *iter;

        const auto args(prg[i][*iter].arguments());
        left.insert(args.begin(), args.end());
      }

      dummy += l.index + l.category;
    }

  std::cout << std::left << std::setw(padding1) << std::setfill('.')
            << "optimized reservoir sampling";
  std::cout << std::right << std::setw(padding2) << std::setfill('.')
            << t.elapsed().count() << "ms\n";

  // -------------------------------------------------------------------------
  t.restart();

  for (unsigned i(0); i < N; ++i)
    for (unsigned j(0); j < N*10; ++j)
    {
      auto l(prg[i].best());
      std::set left({l});

      auto W(random::sup(1.0));

      for (auto iter(left.cbegin()); iter != left.end();)
      {
        for (std::size_t s(std::floor(std::log(random::sup(1.0))
                                      / std::log(1.0-W)) + 1);
             s && iter != left.end();
             --s)
        {
          const auto args(prg[i][*iter].arguments());
          left.insert(args.begin(), args.end());

          ++iter;
        }

        if (iter != left.end())
        {
          l = *iter;
          W *= random::sup(1.0);
        }
      }

      dummy += l.index + l.category;
    }

  std::cout << std::left << std::setw(padding1) << std::setfill('.')
            << "reservoir - algorithm L";
  std::cout << std::right << std::setw(padding2) << std::setfill('.')
            << t.elapsed().count() << "ms\n";

  return dummy;
}

int main()
{
  return static_cast<int>(speed_random_locus());
}
