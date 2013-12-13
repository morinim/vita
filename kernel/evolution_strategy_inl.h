/**
 *
 *  \file evolution_strategy_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_STRATEGY_INL_H)
#define      EVOLUTION_STRATEGY_INL_H

/*
///
/// \param[in] l a layer.
/// \return the maximum allowed age for an individual in layer \a l.
///
template<class T>
unsigned alps_es<T>::max_age(unsigned l) const
{
  assert(l < this->pop_.layers());

  if (l + 1 == this->pop_.layers())
    return std::numeric_limits<unsigned>::max();

  const auto age_gap(this->pop_.env().alps.age_gap);

  // This is a polynomial aging scheme.
  switch (l)
  {
  case 0:   return age_gap;
  case 1:   return age_gap + age_gap;
  default:  return l * l * age_gap;
  }

  // A linear aging scheme.
  // return age_gap * (l + 1);

  // An exponential aging scheme.
  // switch (l)
  // {
  // case 0:  return age_gap;
  // case 1:  return age_gap + age_gap;
  // default:
  // {
  //   auto k(4);
  //   for (unsigned i(2); i < layer; ++i)
  //     k *= 2;
  //   return k * age_gap;
  // }

  // Fibonacci aging scheme.
  // auto num1(age_gap), num2(age_gap);
  // while (num2 <= 2)
  // {
  //   auto num3(num2);
  //   num2 += num1;
  //   num1 = num3;
  // }
  //
  // if (l == 1)
  //   return num1 + num2 - 1;
  //
  // for (unsigned i(1); i <= l; ++i)
  // {
  //   auto num3(num2);
  //   num2 += num1 -1;
  //   num1 = num3;
  // }
  // return num2;
}

///
/// \param[in] c the coordinates of an individual.
/// \return \c true if the individual at coordinates \c is too old for his
///         layer.
///
template<class T>
bool alps_es<T>::aged(coord c) const
{
  return this->pop_[c.layer][c.index].age > max_age(c.layer);
}
*/

///
/// Increments population's age and checks if it's time to add a new layer.
///
template<class T>
void alps_es<T>::post_bookkeeping()
{
  auto &pop(this->pop_);

  pop.inc_age();

  if (this->sum_->gen && this->sum_->gen % pop.env().alps.age_gap == 0)
  {
    if (pop.layers() < pop.env().layers)
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
template<class T>
void alps_es<T>::log(unsigned last_run, unsigned current_run) const
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
        f_lys << std::endl << std::endl;

      for (unsigned l(0); l < pop.layers(); ++l)
      {
        f_lys << current_run << ' ' << this->sum_->gen << ' ' << l << " <";

        if (pop.max_age(l) == std::numeric_limits<unsigned>::max())
          f_lys << "inf";
        else
          f_lys << pop.max_age(l) + 1;

        f_lys << ' ' << this->sum_->az.age_dist(l).mean
              << ' ' << this->sum_->az.age_dist(l).standard_deviation()
              << ' ' << static_cast<unsigned>(this->sum_->az.age_dist(l).min)
              << '-' << static_cast<unsigned>(this->sum_->az.age_dist(l).max)
              << ' ' << this->sum_->az.fit_dist(l).mean
              << ' ' << this->sum_->az.fit_dist(l).standard_deviation()
              << ' ' << this->sum_->az.fit_dist(l).min
              << '-' << this->sum_->az.fit_dist(l).max << std::endl;
      }
    }
  }
}
#endif  // EVOLUTION_STRATEGY_INL_H
