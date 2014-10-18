/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
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
/// Increments population's age and checks if it's time to add a new layer.
///
template<class T,
         template<class> class SS,
         template<class> class CS,
         template<class> class RS>
void basic_alps_es<T, SS, CS, RS>::post_bookkeeping()
{
  const auto &sum(this->sum_);
  auto &pop(this->pop_);

  pop.inc_age();

  const auto layers(pop.layers());
  for (auto l(decltype(layers){1}); l < layers; ++l)
    if (issmall(sum->az.fit_dist(l).standard_deviation()))
    {
      if (pop.individuals(l) / 2 > pop.env().individuals / 10)
        pop.set_allowed(l, pop.individuals(l) / 2);
    }
    else
      pop.set_allowed(l, pop.env().individuals);

  if (sum->gen && sum->gen % pop.env().alps.age_gap == 0)
  {
    if (layers < pop.env().layers)
      pop.add_layer();
    else
    {
      this->replacement.try_move_up_layer(0);
      pop.init_layer(0);
    }
  }
}

///
/// \param[in] last_run last run processed.
/// \param[in] current_run the current run.
///
/// Saves working / statistical informations about layer status.
///
/// Parameters from the environment:
/// * env.stat_layers if \c false the method will not write any data.
///
template<class T,
         template<class> class SS,
         template<class> class CS,
         template<class> class RS>
void basic_alps_es<T, SS, CS, RS>::log(unsigned last_run,
                                       unsigned current_run) const
{
  const auto &pop(this->pop_);
  const auto &env(pop.env());

  if (env.stat_layers)
  {
    const std::string n_lys(env.stat_dir + "/" + environment::lys_filename);
    std::ofstream f_lys(n_lys.c_str(), std::ios_base::app);
    if (f_lys.good())
    {
      if (last_run != current_run)
        f_lys << "\n\n";

      const auto layers(pop.layers());
      for (auto l(decltype(layers){0}); l < layers; ++l)
      {
        f_lys << current_run << ' ' << this->sum_->gen << ' ' << l << " <";

        const auto ma(alps::max_age(l, layers, env.alps.age_gap));
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
}
#endif  // Include guard
