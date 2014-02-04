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
#include <boost/test/unit_test.hpp>

using namespace boost;

#endif

#define TEST_WTA

BOOST_AUTO_TEST_SUITE(lambda)

BOOST_AUTO_TEST_CASE(reg_lambda)
{
  using namespace vita;

  src_problem pr(true);
  auto res(pr.load("mep.csv"));
  BOOST_REQUIRE_EQUAL(res.first, 10);  // mep.csv is a 10 lines file

  BOOST_TEST_CHECKPOINT("REGRESSION TEAM OF ONE INDIVIDUAL");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind(pr.env, pr.sset);
    const reg_lambda_f<individual> li(ind);

    const team<individual> t{{ind}};
    const reg_lambda_f<team<individual>> lt(t);

    for (const auto &e : *pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.empty())
        BOOST_REQUIRE(out_t.empty());
      else
        BOOST_REQUIRE_CLOSE(to<number>(out_i), to<number>(out_t), 0.0001);
    }
  }

  BOOST_TEST_CHECKPOINT("REGRESSION TEAM OF IDENTICAL INDIVIDUALS");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind(pr.env, pr.sset);
    const reg_lambda_f<individual> li(ind);

    const team<individual> t{{ind, ind, ind, ind}};
    const reg_lambda_f<team<individual>> lt(t);

    for (const auto &e : *pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.empty())
        BOOST_REQUIRE(out_t.empty());
      else
        BOOST_REQUIRE_CLOSE(to<number>(out_i), to<number>(out_t), 0.0001);
    }
  }

  BOOST_TEST_CHECKPOINT("REGRESSION TEAM OF RANDOM INDIVIDUALS");
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

    const team<individual> t{{i1, i2, i3, i4}};
    const reg_lambda_f<team<individual>> lambda_team(t);

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

      if (n > 0.0)
      {
        const auto out_t(lambda_team(e));

        if (std::fabs(sum / n) < 0.000001)
          BOOST_REQUIRE_SMALL(to<number>(out_t), 0.000001);
        else
          BOOST_REQUIRE_CLOSE(sum / n, to<number>(out_t), 0.0001);
      }
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

  BOOST_TEST_CHECKPOINT("DYNSLOT LAMBDA TEAM OF ONE INDIVIDUAL");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind(pr.env, pr.sset);
    const dyn_slot_lambda_f<individual> li(ind, *pr.data(), slots);

    const team<individual> t{{ind}};
    const dyn_slot_lambda_f<team<individual>> lt(t, *pr.data(), slots);

    for (const auto &e : *pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.empty())
        BOOST_REQUIRE(out_t.empty());
      else
        BOOST_REQUIRE_EQUAL(li.name(out_i), lt.name(out_t));
    }
  }

  BOOST_TEST_CHECKPOINT("DYNSLOT LAMBDA TEAM OF RANDOM INDIVIDUALS");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind1(pr.env, pr.sset);
    const individual ind2(pr.env, pr.sset);
    const individual ind3(pr.env, pr.sset);

    const dyn_slot_lambda_f<individual> lambda1(ind1, *pr.data(), slots);
    const dyn_slot_lambda_f<individual> lambda2(ind2, *pr.data(), slots);
    const dyn_slot_lambda_f<individual> lambda3(ind3, *pr.data(), slots);

    const team<individual> t{{ind1, ind2, ind3}};
    const auto ts(t.individuals());
    const dyn_slot_lambda_f<team<individual>> lambda_t(t, *pr.data(), slots);

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
      const std::vector<std::pair<class_tag_t, double>> tags =
      {
        lambda1.tag(example), lambda2.tag(example), lambda3.tag(example)
      };

      for (auto j(decltype(ts){0}); j < ts; ++j)
        BOOST_REQUIRE_EQUAL(any_cast<class_tag_t>(out[j]), tags[j].first);

      std::string s_best(names[0]);

#if defined(TEST_MV)
      std::map<std::string, unsigned> votes;

      for (auto j(decltype(ts){0}); j < ts; ++j)
      {
        if (votes.find(names[j]) == votes.end())
          votes[names[j]] = 1;
        else
          ++votes[names[j]];
      }

      unsigned v_best(0);

      for (auto &v : votes)
        if (v.second > v_best)
        {
          s_best = v.first;
          v_best = v.second;
        }
#elif defined(TEST_WTA)
      class_tag_t c_best(0);

      for (auto j(decltype(ts){1}); j < ts; ++j)
        if (tags[j].second > tags[c_best].second)
        {
          s_best = names[j];
          c_best = j;
        }
#endif

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

  BOOST_TEST_CHECKPOINT("GAUSSIAN LAMBDA TEAM OF ONE INDIVIDUAL");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind(pr.env, pr.sset);
    const gaussian_lambda_f<individual> li(ind, *pr.data());

    const team<individual> t{{ind}};
    const gaussian_lambda_f<team<individual>> lt(t, *pr.data());

    for (const auto &e : *pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.empty())
        BOOST_REQUIRE(out_t.empty());
      else
        BOOST_REQUIRE_EQUAL(li.name(out_i), lt.name(out_t));
    }
  }

  BOOST_TEST_CHECKPOINT("GAUSSIAN LAMBDA TEAM OF RANDOM INDIVIDUALS");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind1(pr.env, pr.sset);
    const individual ind2(pr.env, pr.sset);
    const individual ind3(pr.env, pr.sset);

    const gaussian_lambda_f<individual> lambda1(ind1, *pr.data());
    const gaussian_lambda_f<individual> lambda2(ind2, *pr.data());
    const gaussian_lambda_f<individual> lambda3(ind3, *pr.data());

    const team<individual> t{{ind1, ind2, ind3}};
    const auto ts(t.individuals());
    const gaussian_lambda_f<team<individual>> lambda_t(t, *pr.data());

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
      const std::vector<std::pair<class_tag_t, double>> tags =
      {
        lambda1.tag(example), lambda2.tag(example), lambda3.tag(example)
      };

      for (auto j(decltype(ts){0}); j < ts; ++j)
        BOOST_REQUIRE_EQUAL(any_cast<class_tag_t>(out[j]), tags[j].first);

      std::string s_best(names[0]);

#if defined(TEST_MV)
      std::map<std::string, unsigned> votes;

      for (auto j(decltype(ts){0}); j < ts; ++j)
      {
        if (votes.find(names[j]) == votes.end())
          votes[names[j]] = 1;
        else
          ++votes[names[j]];
      }

      unsigned v_best(0);

      for (auto &v : votes)
        if (v.second > v_best)
        {
          s_best = v.first;
          v_best = v.second;
        }
#elif defined(TEST_WTA)
      class_tag_t c_best(0);

      for (auto j(decltype(ts){1}); j < ts; ++j)
        if (tags[j].second > tags[c_best].second)
        {
          s_best = names[j];
          c_best = j;
        }
#endif

      BOOST_REQUIRE_EQUAL(s_best, lambda_t.name(lambda_t(example)));
    }
  }
}

BOOST_AUTO_TEST_CASE(binary_lambda)
{
  using namespace vita;

  src_problem pr(true);
  auto res(pr.load("ionosphere.csv"));
  BOOST_REQUIRE_EQUAL(res.first, 351);

  BOOST_TEST_CHECKPOINT("BINARY LAMBDA TEAM OF ONE INDIVIDUAL");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind(pr.env, pr.sset);
    const binary_lambda_f<individual> li(ind, *pr.data());

    const team<individual> t{{ind}};
    const binary_lambda_f<team<individual>> lt(t, *pr.data());

    for (const auto &e : *pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.empty())
        BOOST_REQUIRE(out_t.empty());
      else
        BOOST_REQUIRE_EQUAL(li.name(out_i), lt.name(out_t));
    }
  }

  BOOST_TEST_CHECKPOINT("BINARY LAMBDA TEAM OF RANDOM INDIVIDUALS");
  for (unsigned i(0); i < 1000; ++i)
  {
    const individual ind1(pr.env, pr.sset);
    const individual ind2(pr.env, pr.sset);
    const individual ind3(pr.env, pr.sset);

    const binary_lambda_f<individual> lambda1(ind1, *pr.data());
    const binary_lambda_f<individual> lambda2(ind2, *pr.data());
    const binary_lambda_f<individual> lambda3(ind3, *pr.data());

    const team<individual> t{{ind1, ind2, ind3}};
    const auto ts(t.individuals());
    const binary_lambda_f<team<individual>> lambda_t(t, *pr.data());

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
      const std::vector<std::pair<class_tag_t, double>> tags =
      {
        lambda1.tag(example), lambda2.tag(example), lambda3.tag(example)
      };

      for (auto j(decltype(ts){0}); j < ts; ++j)
        BOOST_REQUIRE_EQUAL(any_cast<class_tag_t>(out[j]), tags[j].first);

      std::string s_best(names[0]);

#if defined(TEST_MV)
      std::map<std::string, unsigned> votes;

      for (auto j(decltype(ts){0}); j < ts; ++j)
      {
        if (votes.find(names[j]) == votes.end())
          votes[names[j]] = 1;
        else
          ++votes[names[j]];
      }

      unsigned v_best(0);

      for (auto &v : votes)
        if (v.second > v_best)
        {
          s_best = v.first;
          v_best = v.second;
        }
#elif defined(TEST_WTA)
      class_tag_t c_best(0);

      for (auto j(decltype(ts){1}); j < ts; ++j)
        if (tags[j].second > tags[c_best].second)
        {
          s_best = names[j];
          c_best = j;
        }
#endif

      BOOST_REQUIRE_EQUAL(s_best, lambda_t.name(lambda_t(example)));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
