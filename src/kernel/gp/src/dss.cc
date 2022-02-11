/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/dss.h"
#include "kernel/random.h"

namespace vita
{

namespace
{

auto weight(const dataframe::example &v)
{
  return static_cast<std::uintmax_t>(v.difficulty)
         + static_cast<std::uintmax_t>(v.age) * v.age * v.age;
}

}  // unnamed namespace

///
/// Sets up a DSS validator.
///
/// \param[in] prob current problem
/// \param[in] eva_t active training evaluator
/// \param[in] eva_v active validation evaluator
///
/// \note
/// References to the evaluator are required since (possible) cached values
/// must be cleared when changing the training / validation set.
///
dss::dss(src_problem &prob, cached_evaluator &eva_t, cached_evaluator &eva_v)
  : training_(prob.data(dataset_t::training)),
    validation_(prob.data(dataset_t::validation)),
    eva_t_(eva_t), eva_v_(eva_v),
    env_(prob.env)
{
  // Here `!env_.dss.has_value()` could be true. Validation strategy is set
  // before parameters are tuned.
}

void dss::reset_age_difficulty(dataframe &d)
{
  std::for_each(d.begin(), d.end(),
                [](auto &example)
                {
                  example.difficulty = 0;
                  example.age        = 1;
                });
}

std::pair<std::uintmax_t, std::uintmax_t> dss::average_age_difficulty(
  dataframe &d) const
{
  constexpr std::pair<std::uintmax_t, std::uintmax_t> zero(0, 0);

  const auto s(d.size());
  if (!s)
    return zero;

  auto avg(std::accumulate(d.begin(), d.end(), zero,
                           [](const auto &p, const dataframe::example &e)
                           {
                             return std::pair<std::uintmax_t, std::uintmax_t>(
                               p.first + e.age, p.second + e.difficulty);
                           }));

  avg.first  /= s;
  avg.second /= s;

  return avg;
}

void dss::clear_evaluators()
{
  eva_t_.clear();
  eva_v_.clear();
}

void dss::move_to_validation()
{
  std::move(training_.begin(), training_.end(),
            std::back_inserter(validation_));
  training_.clear();

  Ensures(training_.empty());
}

///
/// Available examples are randomly partitioned into two independent sets
/// according to a given percentage.
///
/// \param[in] run current run
///
/// \attention The procedure changes the current training / validation sets.
///
void dss::init(unsigned)
{
  Expects(env_.dss.value_or(0) > 0);

  reset_age_difficulty(training_);
  reset_age_difficulty(validation_);

  shake_impl();
  clear_evaluators();
}

void dss::shake_impl()
{
  Expects(training_.size() + validation_.size() >= 2);

  move_to_validation();

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
  const auto s(static_cast<double>(validation_.size()));
  const double ratio(std::min(0.6, 0.2 + 100.0 / (s + 100.0)));
  assert(0.2 <= ratio && ratio <= 0.6);
  const double target_size(std::max(1.0, s * ratio));
  assert(1.0 <= target_size && target_size <= s);
  const double k(target_size / static_cast<double>(weight_sum));

  auto pivot(
    std::partition(validation_.begin(), validation_.end(),
                   [k](const auto &e)
                   {
                     const auto p1(static_cast<double>(weight(e)) * k);
                     const auto prob(std::min(p1, 1.0));

                     return random::boolean(prob) == false;
                   }));

  if (pivot == validation_.begin() || pivot == validation_.end())
    pivot = std::next(validation_.begin(),
                      static_cast<std::ptrdiff_t>(target_size));

  assert(validation_.size() == static_cast<size_t>(s));
  std::move(pivot, validation_.end(), std::back_inserter(training_));
  validation_.erase(pivot, validation_.end());

  vitaDEBUG << "DSS SHAKE (weight sum: " << weight_sum << ", training with: "
            << training_.size() << ')';
  assert(static_cast<size_t>(s) == training_.size() + validation_.size());

  reset_age_difficulty(training_);

  Ensures(!training_.empty());
  Ensures(!validation_.empty());
}

bool dss::shake(unsigned generation)
{
  Expects(env_.dss.value_or(0) > 0);

  const auto gap(*env_.dss);

  if (generation == 0        // already handled by init()
      || generation % gap)
  {
    assert(!training_.empty());
    assert(!validation_.empty());
    return false;
  }

  vitaDEBUG << "DSS shaking generation " << generation;

  const auto avg_t(average_age_difficulty(training_));
  vitaDEBUG << "DSS average training difficulty " << avg_t.second;
  assert(avg_t.first == 1);

  const auto inc_age([](dataframe::example &e) { ++e.age; });
  std::for_each(training_.begin(), training_.end(), inc_age);
  std::for_each(validation_.begin(), validation_.end(), inc_age);

  shake_impl();
  clear_evaluators();

  return true;
}

///
/// Moves all the example in the validation set.
///
void dss::close(unsigned)
{
  move_to_validation();
  clear_evaluators();
}

}  // namespace vita
