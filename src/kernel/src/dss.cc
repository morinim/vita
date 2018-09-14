/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/src/dss.h"
#include "kernel/random.h"

namespace vita
{

namespace
{

std::uintmax_t weight(const dataframe::example &v)
{
  return v.difficulty + v.age * v.age * v.age;
}

}  // unnamed namespace

dss::dss(src_problem &prob) : training_(prob.data(problem::training)),
                              validation_(prob.data(problem::validation)),
                              gap_(*prob.env.dss)
{
  Expects(*prob.env.dss);
  Expects(validation_.empty());
}

void dss::reset_age_difficulty(dataframe &d)
{
  const auto reset([](dataframe::example &e)
                   {
                     e.difficulty = 0;
                     e.age        = 1;
                   });

  std::for_each(d.begin(), d.end(), reset);
}

std::pair<std::uintmax_t, std::uintmax_t> dss::average_age_difficulty(
  dataframe &d) const
{
  const auto s(d.size());

  const std::pair<std::uintmax_t, std::uintmax_t> zero(0, 0);

  if (!s)
    return zero;

  auto avg(std::accumulate(
             d.begin(), d.end(), zero,
             [](const auto &p, const dataframe::example &e)
             {
               return std::pair<std::uintmax_t, std::uintmax_t>(
                 p.first + e.age, p.second + e.difficulty);
             }));

  avg.first /= s;
  avg.second /= s;

  return avg;
}

void dss::move_to_validation()
{
  std::move(training_.begin(), training_.end(),
            std::back_inserter(validation_));
  training_.clear();
}

///
/// Available examples are randomly partitioned into two independent sets
/// according to a given percentage.
///
/// \attention The procedure resets current training / validation sets.
///
void dss::init()
{
  move_to_validation();

  reset_age_difficulty(validation_);

  shake_impl();
}

void dss::shake_impl()
{
  Expects(training_.empty());
  Expects(!validation_.empty());

  const auto avg_v(average_age_difficulty(validation_));
  vitaDEBUG << "DSS average validation difficulty " << avg_v.second
            << ", age " << avg_v.first;

  const auto weight_sum(
    std::accumulate(validation_.begin(), validation_.end(), std::uintmax_t(0),
                    [](const std::uintmax_t &s, const dataframe::example &e)
                    {
                      return s + weight(e);
                    }));

  assert(weight_sum);

  // Move a subset of the available examples (initially contained in the
  // validation set) into the training set.
  // Note that the actual size of the selected subset is not fixed and, in
  // fact, it averages slightly above `target_size` (Gathercole and Ross felt
  // it might improve performance).
  const double s(validation_.size());
  const auto ratio(std::min(0.6, 0.2 + 100.0 / (s + 100.0)));
  assert(0.2 <= ratio && ratio <= 0.6);
  const auto target_size(s * ratio);
  assert(0.0 <= target_size && target_size <= s);
  const auto k(target_size / static_cast<double>(weight_sum));

  auto pivot(
    std::partition(validation_.begin(), validation_.end(),
                   [k](const auto &e)
                   {
                     const auto p1(static_cast<double>(weight(e)) * k);
                     const auto prob(std::min(p1, 1.0));

                     return random::boolean(prob) == false;
                   }));

  if (pivot == validation_.end())
    pivot = std::next(validation_.begin(), s / 2);

  assert(validation_.size() == s);

  std::move(pivot, validation_.end(), std::back_inserter(training_));
  validation_.erase(pivot, validation_.end());

  vitaDEBUG << "DSS SHAKE (weight sum: " << weight_sum << ", training with: "
            << training_.size() << ')';

  assert(s == training_.size() + validation_.size());

  reset_age_difficulty(training_);
}

bool dss::shake(unsigned generation)
{
  if (generation == 0        // already handled by init()
      || generation % gap_)
    return false;

  vitaDEBUG << "DSS shaking generation " << generation;

  const auto avg_t(average_age_difficulty(training_));
  vitaDEBUG << "DSS average training difficulty " << avg_t.second;
  assert(avg_t.first == 1);

  move_to_validation();

  std::for_each(validation_.begin(), validation_.end(),
                [](auto &e) { ++e.age; });

  shake_impl();

  return true;
}

}  // namespace vita
