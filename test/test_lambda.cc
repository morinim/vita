/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>

#include "kernel/data.h"
#include "kernel/individual.h"
#include "kernel/lambda_f.h"
#include "kernel/team.h"
#include "kernel/src/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE lambda
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

BOOST_AUTO_TEST_SUITE(lambda)

BOOST_AUTO_TEST_CASE(reg_lambda)
{
  using namespace vita;

  src_problem pr;
  auto res(pr.load("mep.csv"));

  for (unsigned i(0); i < 1000; ++i)
  {
    individual ind(pr.env, pr.sset);
    reg_lambda_f<individual> li(ind);

    team<individual> t{{ind, ind, ind, ind}};
    reg_lambda_f<team<individual>> lt(t);

    for (const auto &e : *pr.data())
    {
      const auto ai(li(e)), at(lt(e));

      if (ai.empty())
        BOOST_REQUIRE(at.empty());
      else
        BOOST_REQUIRE_CLOSE(to<number>(ai), to<number>(at), 0.0001);
    }
  }

  for (unsigned i(0); i < 1000; ++i)
  {
    std::vector<individual> ind =
    {
      individual(pr.env, pr.sset),
      individual(pr.env, pr.sset),
      individual(pr.env, pr.sset),
      individual(pr.env, pr.sset)
    };

    std::vector<reg_lambda_f<individual>> li =
    {
      reg_lambda_f<individual>(ind[0]),
      reg_lambda_f<individual>(ind[1]),
      reg_lambda_f<individual>(ind[2]),
      reg_lambda_f<individual>(ind[3])
    };

    team<individual> t(ind);
    reg_lambda_f<team<individual>> lt(t);

    for (const auto &e : *pr.data())
    {
      const std::vector<vita::any> ai{li[0](e), li[1](e), li[2](e), li[3](e)};
      const auto at(lt(e));

      if (ai[0].empty() || ai[1].empty() || ai[2].empty() || ai[3].empty())
        BOOST_REQUIRE(at.empty());
      else
      {
        const number n(to<number>(ai[0]) + to<number>(ai[1]) +
                       to<number>(ai[2]) + to<number>(ai[3]));
        BOOST_REQUIRE_CLOSE(n / 4.0, to<number>(at), 0.0001);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(dyn_slot_lambda)
{
  using namespace vita;

  const unsigned slots(10);

  src_problem pr;
  auto res(pr.load("iris.csv"));

  for (unsigned i(0); i < 100; ++i)
  {
    std::vector<individual> ind =
    {
      individual(pr.env, pr.sset), individual(pr.env, pr.sset),
      individual(pr.env, pr.sset)
    };

    std::vector<dyn_slot_lambda_f<individual>> li =
    {
      dyn_slot_lambda_f<individual>(ind[0], *pr.data(), slots),
      dyn_slot_lambda_f<individual>(ind[1], *pr.data(), slots),
      dyn_slot_lambda_f<individual>(ind[2], *pr.data(), slots)
    };

    team<individual> t(ind);
    dyn_slot_lambda_f<team<individual>> lt(t, *pr.data(), slots);

    for (const auto &example : *pr.data())
    {
      const std::vector<vita::any> ai =
        {li[0](example), li[1](example), li[2](example)};
      const vita::any at(lt(example));

      if (ai[0].empty() || ai[1].empty() || ai[2].empty())
        continue;
      else
      {
        std::map<std::string, unsigned> votes;

        for (unsigned i(0); i < ai.size(); ++i)
        {
          const std::string name(li[i].name(ai[i]));
          if (votes.find(name) == votes.end())
            votes[name] = 1;
          else
            ++votes[name];
        }

        std::string s_best;
        unsigned c_best(0);

        for (auto &i : votes)
          if (i.second > c_best)
            s_best = i.first;

        BOOST_REQUIRE_EQUAL(s_best, lt.name(at));
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
