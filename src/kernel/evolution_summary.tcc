/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_SUMMARY_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_SUMMARY_TCC)
#define      VITA_EVOLUTION_SUMMARY_TCC

///
/// Default constructor just call the summary::clear method.
///
template<class T>
summary<T>::summary()
{
  clear();
}

///
/// Resets summary informations.
///
template<class T>
void summary<T>::clear()
{
  az.clear();

  best = boost::none;

  elapsed    = 0.0;
  mutations  = 0;
  crossovers = 0;
  gen        = 0;
  last_imp   = 0;
}

///
/// \param[in] in input stream.
/// \param[in] e an environment (needed to build the best individual).
/// \param[in] s a symbol_set (needed to build the best individual).
/// \return `true` if the object loaded correctly.
///
/// \note
/// If the load operation isn't successful the current object isn't changed.
///
template<class T>
bool summary<T>::load(std::istream &in, const environment &e,
                      const symbol_set &s)
{
  unsigned known_best(false);
  if (!(in >> known_best))
    return false;

  summary tmp_summary;
  if (known_best)
  {
    T tmp_ind(e, s);
    if (!tmp_ind.load(in))
      return false;

    decltype(best->fitness) tmp_fitness;
    if (!tmp_fitness.load(in))
      return false;

    tmp_summary.best = {tmp_ind, tmp_fitness};
  }
  else
    tmp_summary.best = boost::none;

  if (!(in >> tmp_summary.elapsed >> tmp_summary.mutations
           >> tmp_summary.crossovers >> tmp_summary.gen
           >> tmp_summary.last_imp))
    return false;

  *this = tmp_summary;
  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if summary was saved correctly.
///
template<class T>
bool summary<T>::save(std::ostream &out) const
{
  // analyzer az doesn't need to be saved: it'll be recalculated at the
  // beginning of evolution.

  if (best)
  {
    out << "1\n";
    best->ind.save(out);
    best->fitness.save(out);
  }
  else
    out << "0\n";

  out << elapsed << ' ' << mutations << ' ' << crossovers << ' ' << gen << ' '
      << last_imp << '\n';

  return out.good();
}

#endif  // Include guard
