/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2019 EOS di Manlio Morini.
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
summary<T>::summary() : az(), best{T(), model_measurements()}, elapsed(0),
                        crossovers(0), mutations(0), gen(0), last_imp(0)
{
}

///
/// Resets summary informations.
///
template<class T>
void summary<T>::clear()
{
  *this = summary<T>();
}

///
/// Loads the object from a stream.
///
/// \param[in] in input stream
/// \param[in] p  active problem
/// \return       `true` if the object loaded correctly
///
/// \note
/// If the load operation isn't successful the current object isn't changed.
///
template<class T>
bool summary<T>::load(std::istream &in, const problem &p)
{
  unsigned known_best(false);
  if (!(in >> known_best))
    return false;

  summary tmp_summary;
  if (known_best)
  {
    T tmp_ind;
    if (!tmp_ind.load(in, p.sset))
      return false;

    decltype(best.score.fitness) tmp_fitness;
    if (!tmp_fitness.load(in))
      return false;

    decltype(best.score.accuracy) tmp_accuracy;
    if (!load_float_from_stream(in, &tmp_accuracy))
      return false;

    tmp_summary.best.solution = tmp_ind;
    tmp_summary.best.score.fitness = tmp_fitness;
    tmp_summary.best.score.accuracy = tmp_accuracy;
  }

  int ms;
  if (!(in >> ms))
    return false;
  tmp_summary.elapsed = std::chrono::milliseconds(ms);

  if (!(in >> tmp_summary.mutations  >> tmp_summary.crossovers
           >> tmp_summary.gen >> tmp_summary.last_imp))
    return false;

  *this = tmp_summary;
  return true;
}

///
/// Saves the object into a stream.
///
/// \param[out] out output stream
/// \return     `true` if summary was saved correctly
///
template<class T>
bool summary<T>::save(std::ostream &out) const
{
  // analyzer az doesn't need to be saved: it'll be recalculated at the
  // beginning of evolution.

  if (best.solution.empty())
    out << "0\n";
  else
  {
    out << "1\n";
    best.solution.save(out);
    best.score.fitness.save(out);
    save_float_to_stream(out, best.score.accuracy);
    out << '\n';
  }

  out << elapsed.count() << ' ' << mutations << ' ' << crossovers << ' '
      << gen << ' ' << last_imp << '\n';

  return out.good();
}

#endif  // include guard
