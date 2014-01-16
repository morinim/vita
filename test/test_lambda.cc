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

  src_problem pr(true);
  auto res(pr.load("mep.csv"));
  BOOST_REQUIRE_EQUAL(res.first, 10);  // mep.csv is a 10 lines file

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
    const individual i1(pr.env, pr.sset);
    const individual i2(pr.env, pr.sset);
    const individual i3(pr.env, pr.sset);
    const individual i4(pr.env, pr.sset);

    const reg_lambda_f<individual> lambda1(i1);
    const reg_lambda_f<individual> lambda2(i2);
    const reg_lambda_f<individual> lambda3(i3);
    const reg_lambda_f<individual> lambda4(i4);

    team<individual> t{{i1, i2, i3, i4}};
    reg_lambda_f<team<individual>> lambda_team(t);

    for (const auto &e : *pr.data())
    {
      const auto out1(lambda1(e));
      const auto out2(lambda2(e));
      const auto out3(lambda3(e));
      const auto out4(lambda4(e));

      number sum(0.0), n(0.0);
      if (!out1.empty())
      {
        sum += to<number>(out1);
        ++n;
      }
      if (!out2.empty())
      {
        sum += to<number>(out2);
        ++n;
      }
      if (!out3.empty())
      {
        sum += to<number>(out3);
        ++n;
      }
      if (!out4.empty())
      {
        sum += to<number>(out4);
        ++n;
      }

      BOOST_REQUIRE_CLOSE(sum / n, to<number>(lambda_team(e)), 0.0001);
    }
  }
}

BOOST_AUTO_TEST_CASE(dyn_slot_lambda)
{
  using namespace vita;

  const unsigned slots(10);

  src_problem pr(true);
  auto res(pr.load("iris.csv"));
  BOOST_REQUIRE_EQUAL(res.first, 150);

  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind1(pr.env, pr.sset);
    const individual ind2(pr.env, pr.sset);
    const individual ind3(pr.env, pr.sset);

    const dyn_slot_lambda_f<individual> lambda1(ind1, *pr.data(), slots);
    const dyn_slot_lambda_f<individual> lambda2(ind2, *pr.data(), slots);
    const dyn_slot_lambda_f<individual> lambda3(ind3, *pr.data(), slots);

    team<individual> t{{ind1, ind2, ind3}};
    dyn_slot_lambda_f<team<individual>> lambda_t(t, *pr.data(), slots);

    for (const auto &example : *pr.data())
    {
      const std::vector<vita::any> out =
      {
        lambda1(example), lambda2(example), lambda3(example)
      };
      const std::vector<std::string> names =
      {
        lambda1.name(out[0]), lambda2.name(out[1]), lambda3.name(out[2])
      };

      std::map<std::string, unsigned> votes;

      for (unsigned i(0); i < out.size(); ++i)
      {
        if (votes.find(names[i]) == votes.end())
          votes[names[i]] = 1;
        else
          ++votes[names[i]];
      }

      std::string s_best;
      unsigned c_best(0);

      for (auto &i : votes)
        if (i.second > c_best)
        {
          s_best = i.first;
          c_best = i.second;
        }

      BOOST_REQUIRE_EQUAL(s_best, lambda_t.name(lambda_t(example)));
    }
  }
}

BOOST_AUTO_TEST_CASE(gaussian_lambda)
{
  using namespace vita;

  src_problem pr(true);
  auto res(pr.load("iris.csv"));
  BOOST_REQUIRE_EQUAL(res.first, 150);

  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind1(pr.env, pr.sset);
    const individual ind2(pr.env, pr.sset);
    const individual ind3(pr.env, pr.sset);

    const gaussian_lambda_f<individual> lambda1(ind1, *pr.data());
    const gaussian_lambda_f<individual> lambda2(ind2, *pr.data());
    const gaussian_lambda_f<individual> lambda3(ind3, *pr.data());

    team<individual> t{{ind1, ind2, ind3}};
    gaussian_lambda_f<team<individual>> lambda_t(t, *pr.data());

    for (const auto &example : *pr.data())
    {
      const std::vector<vita::any> out =
      {
        lambda1(example), lambda2(example), lambda3(example)
      };
      const std::vector<std::string> names =
      {
        lambda1.name(out[0]), lambda2.name(out[1]), lambda3.name(out[2])
      };

      std::map<std::string, unsigned> votes;

      for (unsigned i(0); i < out.size(); ++i)
      {
        if (votes.find(names[i]) == votes.end())
          votes[names[i]] = 1;
        else
          ++votes[names[i]];
      }

      std::string s_best;
      unsigned c_best(0);

      for (auto &i : votes)
        if (i.second > c_best)
        {
          s_best = i.first;
          c_best = i.second;
        }

      BOOST_REQUIRE_EQUAL(s_best, lambda_t.name(lambda_t(example)));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
