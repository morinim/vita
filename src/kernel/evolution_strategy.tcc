/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
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
/// - all the individuals have the same fitness;
/// - after `env_.g_without_improvement` generations the situation doesn't
///   change.
///
template<class T>
bool std_es<T>::stop_condition() const
{
  const auto &env(this->pop_.env());

  if (env.g_without_improvement &&
      this->sum_->gen - this->sum_->last_imp > env.g_without_improvement &&
      issmall(this->sum_->az.fit_dist().variance()))
    return true;

  return false;
}

///
/// \param[out] env environemnt
/// \return         a strategy-specific environment
///
/// \remark ALPS need more than one layer.
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
void basic_alps_es<T, CS>::post_bookkeeping()
{
  const auto &sum(this->sum_);
  auto &pop(this->pop_);

  pop.inc_age();

  const auto layers(pop.layers());
  for (auto l(decltype(layers){1}); l < layers; ++l)
  {
    const auto allowed(pop.env().individuals);
    const auto current(pop.individuals(l));
    if (issmall(sum->az.fit_dist(l).standard_deviation()))
    {
      if (current > allowed / 5)
        pop.set_allowed(l, current / 2);
    }
    else
      pop.set_allowed(l, allowed);
  }

  // Code executed every `age_gap` interval.
  if (sum->gen && sum->gen % pop.env().alps.age_gap == 0)
  {
    if (layers < pop.env().layers ||
        sum->az.age_dist(layers - 1).mean() >
        alps::max_age(layers, pop.env().alps.age_gap))
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
/// * env.stat.layers if `false` the method will not write any data.
///
template<class T, template<class> class CS>
void basic_alps_es<T, CS>::log(unsigned last_run, unsigned current_run) const
{
  const auto &pop(this->pop_);
  const auto &env(pop.env());

  if (env.stat.layers)
  {
    const std::string n_lys(env.stat.dir + "/" + env.stat.lys_name);
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
