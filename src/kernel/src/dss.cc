/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
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

std::uintmax_t weight(const src_data::example &v)
{
  return v.difficulty + v.age * v.age * v.age;
}

}  // namespace

dss::dss(src_data &d) : dat_(d)
{
}

void dss::reset_age_difficulty(data::dataset_t d)
{
  Expects(d != data::npos);

  std::for_each(dat_.begin(d), dat_.end(d),
                [](src_data::example &e)
                {
                  e.difficulty = 0;
                  e.age        = 1;
                });
}

std::pair<uintmax_t, uintmax_t> dss::average_age_difficulty(
  data::dataset_t d) const
{
  const auto s(dat_.size(d));

  if (!s)
    return {0, 0};

  auto avg(std::accumulate(
             dat_.begin(d), dat_.end(d),
             std::pair<uintmax_t, uintmax_t>(0, 0),
             [](const std::pair<uintmax_t, uintmax_t> &p,
                const src_data::example &e)
             {
               return std::pair<uintmax_t, uintmax_t>(p.first + e.age,
                                                      p.second + e.difficulty);
             }));

  avg.first /= s;
  avg.second /= s;

  return avg;
}

///
/// Available examples are randomly partitioned into two independent sets
/// according to a given percentage.
///
/// \attention The procedure resets current training / validation sets.
///
void dss::preliminary_setup()
{
  dat_.move_append(data::training, data::validation);

  reset_age_difficulty(data::validation);

  shake_impl();
}

void dss::shake_impl()
{
  Expects(dat_.size(data::validation));
  Expects(dat_.size(data::training) == 0);

  const auto avg_v(average_age_difficulty(data::validation));
  print.debug("DSS average validation difficulty ", avg_v.second, ", age ",
              avg_v.first);

  const auto weight_sum(
    std::accumulate(dat_.begin(data::validation), dat_.end(data::validation),
                    uintmax_t(0),
                    [](const uintmax_t &s, const src_data::example &e)
                    {
                      return s + weight(e);
                    }));

  assert(weight_sum);

  // Move a subset of the available examples (currently contained in the
  // validation set) into the training set.
  // Note that the actual size of the selected subset is not fixed and, in
  // fact, it averages slightly above target_size (Gathercole and Ross felt
  // that this might improve performance).
  const double s(dat_.size(data::validation));
  const auto ratio(std::min(0.6, 0.2 + 100.0 / (s + 100.0)));
  assert(0.2 <= ratio && ratio <= 0.6);
  const auto target_size(s * ratio);
  assert(0.0 <= target_size && target_size <= s);
  const auto k(target_size / static_cast<double>(weight_sum));

  auto pivot(
    std::partition(dat_.begin(data::validation), dat_.end(data::validation),
                   [=](const src_data::example &e)
                   {
                     const auto p1(static_cast<double>(weight(e)) * k);
                     const auto prob(std::min(p1, 1.0));

                     return random::boolean(prob) == false;
                   }));

  const auto d(static_cast<std::size_t>(
                 std::distance(pivot, dat_.end(data::validation))));
  const std::size_t n(d < 10 ? 10 : d);
  dat_.move_append(data::validation, data::training, n);

  print.debug("DSS SHAKE (weight sum: ", weight_sum, ", training with: ", n,
              ")");

  reset_age_difficulty(data::training);
}

bool dss::shake(unsigned generation)
{
  if (generation == 0 ||    // already handled by preliminary_setup()
      generation % 4 != 0)
    return false;

  print.debug("DSS shaking generation ", generation);

  const auto avg_t(average_age_difficulty(data::training));
  print.debug("DSS average training difficulty ", avg_t.second);
  assert(avg_t.first == 1);

  dat_.move_append(data::training, data::validation);

  std::for_each(dat_.begin(data::validation), dat_.end(data::validation),
                [](src_data::example &e) { ++e.age; });

  shake_impl();

  return true;
}

}  // namespace vita
