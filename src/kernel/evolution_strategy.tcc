/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_STRATEGY_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_STRATEGY_TCC)
#define      VITA_EVOLUTION_STRATEGY_TCC

///
/// \param[out] env environment
/// \return         a strategy-specific environment
///
/// \remark For standard evolution we only need one layer.
///
template<class T>
environment std_es<T>::shape(environment env)
{
  env.layers = 1;
  return env;
}

///
/// \return `true` when evolution must be stopped
///
/// We use an accelerated stop condition when:
/// - after `max_stuck_time` generations the situation doesn't change;
/// - all the individuals have the same fitness.
///
template<class T>
bool std_es<T>::stop_condition() const
{
  const auto &env(this->pop_.get_problem().env);
  Expects(env.max_stuck_time.has_value());

  const auto &sum(this->sum_);
  Expects(sum->gen >= sum->last_imp);

  // Pay attention to `env.max_stuck_time`: it can be a large number and cause
  // overflow. E.g. `sum->gen > sum->last_imp + *env.max_stuck_time`

  if (sum->gen - sum->last_imp > *env.max_stuck_time
      && issmall(sum->az.fit_dist().variance()))
    return true;

  return false;
}

///
/// \param[out] env environemnt
/// \return         a strategy-specific environment
///
/// \remark ALPS requires more than one layer.
///
template<class T, template<class> class CS>
environment basic_alps_es<T, CS>::shape(environment env)
{
  env.layers = 4;
  return env;
}

///
/// Increments population's age and checks if it's time to add a new layer.
///
template<class T, template<class> class CS>
void basic_alps_es<T, CS>::after_generation()
{
  const auto &sum(this->sum_);
  auto &pop(this->pop_);
  const auto &env(pop.get_problem().env);

  pop.inc_age();

  for (auto l(pop.layers() - 1); l; --l)
    if (almost_equal(sum->az.fit_dist(l - 1).mean(),
                     sum->az.fit_dist(    l).mean()))
      pop.remove_layer(l);

  auto layers(pop.layers());

  for (decltype(layers) l(1); l < layers; ++l)
    if (issmall(sum->az.fit_dist(l).standard_deviation()))
    {
      const auto current(pop.individuals(l));
      pop.set_allowed(l, std::max(env.min_individuals, current / 2));
    }
    else
    {
      const auto allowed(env.individuals);
      pop.set_allowed(l, allowed);
    }

  // Code executed every `age_gap` interval.
  if (sum->gen && sum->gen % env.alps.age_gap == 0)
  {
    if (layers < env.layers ||
        sum->az.age_dist(layers - 1).mean() >
        alps::max_age(layers, env.alps.age_gap))
      pop.add_layer();
    else
    {
      this->replacement.try_move_up_layer(0);
      pop.init_layer(0);
    }
  }
}

///
/// Saves working / statistical informations about layer status.
///
/// \param[in] last_run    last run processed
/// \param[in] current_run current run
///
/// Parameters from the environment:
/// * `env.stat.layers_file` if empty the method will not write any data.
///
template<class T, template<class> class CS>
void basic_alps_es<T, CS>::log_strategy(unsigned last_run,
                                        unsigned current_run) const
{
  const auto &pop(this->pop_);
  const auto &env(pop.get_problem().env);

  if (!env.stat.layers_file.empty())
  {
    const auto n_lys(env.stat.dir / env.stat.layers_file);
    std::ofstream f_lys(n_lys, std::ios_base::app);
    if (!f_lys.good())
      return;

    if (last_run != current_run)
      f_lys << "\n\n";

    auto layers(pop.layers());
    for (decltype(layers) l(0); l < layers; ++l)
    {
      f_lys << current_run << ' ' << this->sum_->gen << ' ' << l << " <";

      const auto ma(alps::allowed_age(pop, l));
      if (ma == std::numeric_limits<decltype(ma)>::max())
        f_lys << "inf";
      else
        f_lys << ma + 1;

      f_lys << ' ' << this->sum_->az.age_dist(l).mean()
            << ' ' << this->sum_->az.age_dist(l).standard_deviation()
            << ' ' << static_cast<unsigned>(this->sum_->az.age_dist(l).min())
            << '-' << static_cast<unsigned>(this->sum_->az.age_dist(l).max())
            << ' ' << this->sum_->az.fit_dist(l).mean()
            << ' ' << this->sum_->az.fit_dist(l).standard_deviation()
            << ' ' << this->sum_->az.fit_dist(l).min()
            << '-' << this->sum_->az.fit_dist(l).max()
            << ' ' << pop.individuals(l) << '\n';
    }
  }
}
#endif  // include guard
