/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>

#include "kernel/i_mep.h"
#include "kernel/lambda_f.h"
#include "kernel/team.h"
#include "kernel/src/problem.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE lambda
#include <boost/test/unit_test.hpp>

using namespace boost;

constexpr double epsilon(0.00001);
#endif

#define TEST_WTA

// Examples in dataset
const std::size_t MEP_COUNT        =  10;
const std::size_t IRIS_COUNT       = 150;
const std::size_t IONOSPHERE_COUNT = 351;

template<class T> using reg_model = vita::basic_reg_lambda_f<T, true>;

BOOST_AUTO_TEST_SUITE(lambda)

template<template<class> class L, class T, unsigned P>
struct build
{
  L<T> operator()(const T &prg, vita::dataframe &d) const
  {
    return L<T>(prg, d, P);
  }
};

template<template<class> class L, class T>
struct build<L, T, 0>
{
  L<T> operator()(const T &prg, vita::dataframe &d) const
  {
    return L<T>(prg, d);
  }
};

template<class T>
struct build<reg_model, T, 0>
{
  reg_model<T> operator()(const T &prg, vita::dataframe &) const
  {
    return reg_model<T>(prg);
  }
};

template<template<class> class L, class T, unsigned P = 0>
void test_serialization(vita::src_problem &pr)
{
  using namespace vita;

  for (unsigned k(0); k < 256; ++k)
  {
    const T ind(pr);
    const auto lambda1(build<L, T, P>()(ind, pr.data()));

    std::stringstream ss;

    BOOST_TEST(lambda1.save(ss));
    const T ind2(pr);
    auto lambda2(build<L, T, P>()(ind2, pr.data()));
    BOOST_TEST(lambda2.load(ss, pr));
    BOOST_TEST(lambda2.debug());

    for (const auto &e : pr.data())
    {
      const auto out1(lambda1.name(lambda1(e)));
      const auto out2(lambda2.name(lambda2(e)));

      BOOST_TEST(out1 == out2);
    }
  }
}

template<template<class> class L, unsigned P = 0>
void test_team_of_one(vita::src_problem &pr)
{
  using namespace vita;

  for (unsigned i(0); i < 1000; ++i)
  {
    const i_mep ind(pr);
    const auto li(build<L, i_mep, P>()(ind, pr.data()));

    const team<i_mep> t{{ind}};
    const auto lt(build<L, team<i_mep>, P>()(t, pr.data()));

    for (const auto &e : pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.has_value())
      {
        const auto v1(to<number>(out_i));
        const auto v2(to<number>(out_t));

        BOOST_TEST(v1 == v2, boost::test_tools::tolerance(epsilon));
      }
      else
        BOOST_TEST(!out_t.has_value());
    }
  }
}

BOOST_AUTO_TEST_CASE(reg_lambda, * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;

  src_problem pr(initialization::standard);

  BOOST_TEST(pr.load("./test_resources/mep.csv").first == MEP_COUNT);

  BOOST_TEST_CHECKPOINT("REGRESSION TEAM OF ONE INDIVIDUAL");
  test_team_of_one<reg_model>(pr);

  BOOST_TEST_CHECKPOINT("REGRESSION TEAM OF IDENTICAL INDIVIDUALS");
  for (unsigned i(0); i < 1000; ++i)
  {
    const i_mep ind(pr);
    const reg_model<i_mep> li(ind);

    const team<i_mep> t{{ind, ind, ind, ind}};
    const reg_model<team<i_mep>> lt(t);

    for (const auto &e : pr.data())
    {
      const auto out_i(li(e)), out_t(lt(e));

      if (out_i.has_value())
      {
        const auto v1(to<number>(out_i));
        const auto v2(to<number>(out_t));

        BOOST_TEST(v1 == v2);
      }
      else
        BOOST_TEST(!out_t.has_value());
    }
  }

  BOOST_TEST_CHECKPOINT("REGRESSION TEAM OF RANDOM INDIVIDUALS");
  for (unsigned i(0); i < 1000; ++i)
  {
    const i_mep i1(pr);
    const i_mep i2(pr);
    const i_mep i3(pr);
    const i_mep i4(pr);

    const reg_model<i_mep> lambda1(i1);
    const reg_model<i_mep> lambda2(i2);
    const reg_model<i_mep> lambda3(i3);
    const reg_model<i_mep> lambda4(i4);

    const team<i_mep> t{{i1, i2, i3, i4}};
    const reg_model<team<i_mep>> lambda_team(t);

    for (const auto &e : pr.data())
    {
      const auto out1(lambda1(e));
      const auto out2(lambda2(e));
      const auto out3(lambda3(e));
      const auto out4(lambda4(e));

      number sum(0.0), n(0.0);
      if (out1.has_value())
      {
        sum += to<number>(out1);
        ++n;
      }
      if (out2.has_value())
      {
        sum += to<number>(out2);
        ++n;
      }
      if (out3.has_value())
      {
        sum += to<number>(out3);
        ++n;
      }
      if (out4.has_value())
      {
        sum += to<number>(out4);
        ++n;
      }

      if (n > 0.0)
      {
        const auto out_t(lambda_team(e));

        if (std::fabs(sum / n) < 0.000001)
          BOOST_TEST(to<number>(out_t) == 0.0);
        else
          BOOST_TEST(sum / n == to<number>(out_t));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(reg_lambda_serialization,
                     * boost::unit_test::tolerance(epsilon))
{
  using namespace vita;

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/mep.csv").first == MEP_COUNT);

  for (unsigned k(0); k < 1000; ++k)
  {
    const i_mep ind(pr);
    const reg_model<i_mep> lambda1(ind);

    std::stringstream ss;

    BOOST_TEST(lambda1.save(ss));
    const i_mep ind2(pr);
    reg_model<i_mep> lambda2(ind2);
    BOOST_TEST(lambda2.load(ss, pr));
    BOOST_TEST(lambda2.debug());

    for (const auto &e : pr.data())
    {
      const auto out1(lambda1(e));
      const auto out2(lambda2(e));

      if (out1.has_value())
        BOOST_TEST(to<number>(out1) == to<number>(out2));
      else
        BOOST_TEST(!out2.has_value());
    }
  }
}

template<template<class> class L, unsigned P = 0>
void test_team(vita::src_problem &pr)
{
  using namespace vita;

  for (unsigned i(0); i < 1000; ++i)
  {
    const i_mep ind1(pr);
    const i_mep ind2(pr);
    const i_mep ind3(pr);

    const auto lambda1(build<L, i_mep, P>()(ind1, pr.data()));
    const auto lambda2(build<L, i_mep, P>()(ind2, pr.data()));
    const auto lambda3(build<L, i_mep, P>()(ind3, pr.data()));

    const team<i_mep> t{{ind1, ind2, ind3}};
    const auto ts(t.individuals());
    const auto lambda_t(build<L, team<i_mep>, P>()(t, pr.data()));

    for (const auto &example : pr.data())
    {
      const std::vector<vita::any> out =
      {
        lambda1(example), lambda2(example), lambda3(example)
      };
      const std::vector<std::string> names =
      {
        lambda1.name(out[0]), lambda2.name(out[1]), lambda3.name(out[2])
      };
      const std::vector<std::pair<class_t, double>> tags =
      {
        lambda1.tag(example), lambda2.tag(example), lambda3.tag(example)
      };

      for (auto j(decltype(ts){0}); j < ts; ++j)
        BOOST_TEST(any_cast<class_t>(out[j]) == tags[j].first);

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
      class_t c_best(0);

      for (auto j(decltype(ts){1}); j < ts; ++j)
        if (tags[j].second > tags[c_best].second)
        {
          s_best = names[j];
          c_best = j;
        }
#endif

      BOOST_TEST(s_best == lambda_t.name(lambda_t(example)));
    }
  }
}

BOOST_AUTO_TEST_CASE(dyn_slot_lambda)
{
  using namespace vita;

  constexpr unsigned slots(10);

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/iris.csv").first == IRIS_COUNT);

  BOOST_TEST_CHECKPOINT("DYNSLOT LAMBDA TEAM OF ONE INDIVIDUAL");
  test_team_of_one<dyn_slot_lambda_f, slots>(pr);

  BOOST_TEST_CHECKPOINT("DYNSLOT LAMBDA TEAM OF RANDOM INDIVIDUALS");
  test_team<dyn_slot_lambda_f, slots>(pr);
}

BOOST_AUTO_TEST_CASE(dyn_slot_lambda_serialization)
{
  using namespace vita;

  constexpr unsigned slots(10);

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/iris.csv").first == IRIS_COUNT);

  BOOST_TEST_CHECKPOINT("DYN_SLOT_LAMBDA_F SERIALIZATION - INDIVIDUAL");
  test_serialization<dyn_slot_lambda_f, i_mep, slots>(pr);

  BOOST_TEST_CHECKPOINT("DYN_SLOT_LAMBDA_F SERIALIZATION - TEAM");
  test_serialization<dyn_slot_lambda_f, team<i_mep>, slots>(pr);
}

BOOST_AUTO_TEST_CASE(gaussian_lambda)
{
  using namespace vita;

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/iris.csv").first == IRIS_COUNT);

  BOOST_TEST_CHECKPOINT("GAUSSIAN LAMBDA TEAM OF ONE INDIVIDUAL");
  test_team_of_one<gaussian_lambda_f>(pr);

  BOOST_TEST_CHECKPOINT("GAUSSIAN LAMBDA TEAM OF RANDOM INDIVIDUALS");
  test_team<gaussian_lambda_f>(pr);
}

BOOST_AUTO_TEST_CASE(gaussian_lambda_serialization)
{
  using namespace vita;

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/iris.csv").first == IRIS_COUNT);

  BOOST_TEST_CHECKPOINT("GAUSSIAN_LAMBDA_F SERIALIZATION - INDIVIDUAL");
  test_serialization<gaussian_lambda_f, i_mep>(pr);

  BOOST_TEST_CHECKPOINT("GAUSSIAN_LAMBDA_F SERIALIZATION - TEAM");
  test_serialization<gaussian_lambda_f, team<i_mep>>(pr);
}

BOOST_AUTO_TEST_CASE(binary_lambda)
{
  using namespace vita;

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/ionosphere.csv").first
             == IONOSPHERE_COUNT);

  BOOST_TEST_CHECKPOINT("BINARY LAMBDA TEAM OF ONE INDIVIDUAL");
  test_team_of_one<binary_lambda_f>(pr);

  BOOST_TEST_CHECKPOINT("BINARY LAMBDA TEAM OF RANDOM INDIVIDUALS");
  test_team<binary_lambda_f>(pr);
}

BOOST_AUTO_TEST_CASE(binary_lambda_serialization)
{
  using namespace vita;

  src_problem pr(initialization::standard);
  BOOST_TEST(pr.load("./test_resources/ionosphere.csv").first
             == IONOSPHERE_COUNT);

  BOOST_TEST_CHECKPOINT("BINARY_LAMBDA_F SERIALIZATION - INDIVIDUAL");
  test_serialization<binary_lambda_f, i_mep>(pr);

  BOOST_TEST_CHECKPOINT("BINARY_LAMBDA_F SERIALIZATION - TEAM");
  test_serialization<binary_lambda_f, team<i_mep>>(pr);
}

BOOST_AUTO_TEST_SUITE_END()
