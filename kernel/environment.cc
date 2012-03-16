/**
 *
 *  \file environment.cc
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>

#include "kernel/environment.h"
#include "kernel/terminal.h"

namespace vita
{
  const char environment::arl_filename[] =     "arl";
  const char environment::dyn_filename[] = "dynamic";
  const char environment::sum_filename[] = "summary";

  ///
  /// Class constructor. Default values are quite reasonable, but specific
  /// problems need ad-hoc parameters.
  ///
  environment::environment()
    : elitism(boost::indeterminate),
      dss(boost::indeterminate),
      g_without_improvement(0),
      arl(true),
      ttable_size(16),
      stat_dir(""),
      stat_arl(false), stat_dynamic(false), stat_summary(false)
  {
    assert(check(false));
  }

  ///
  /// \param[out] pt output tree.
  /// \param[in] path \a path where to store the environment data file.
  ///
  /// Saves the informations regarding the environment using the \a pt boost
  /// property tree.
  ///
  void environment::log(boost::property_tree::ptree *const pt,
                        const std::string &path) const
  {
    assert(stat_summary);

    const std::string env(path + "environment.");
    pt->put(env + "population_size", individuals);
    pt->put(env + "code_length", code_length);
    pt->put(env + "elitism", elitism);
    pt->put(env + "mutation_rate", p_mutation);
    pt->put(env + "crossover_rate", p_cross);
    pt->put(env + "brood_recombination", brood_recombination);
    pt->put(env + "dss", dss);
    pt->put(env + "parent_tournament_size", par_tournament);
    pt->put(env + "replacement_tournament_size", rep_tournament);
    pt->put(env + "mating_zone", mate_zone);
    pt->put(env + "max_gens_since_start", g_since_start);
    pt->put(env + "max_gens_wo_imp", g_without_improvement);
    pt->put(env + "arl", arl);
    pt->put(env + "ttable_bits", ttable_size);  // size 1u << ttable_size.
    pt->put(env + "statistics.directory", stat_dir);
    pt->put(env + "statistics.save_arl", stat_arl);
    pt->put(env + "statistics.save_dynamics", stat_dynamic);
    pt->put(env + "statistics.save_summary", stat_summary);
  }

  ///
  /// \param[in] i pointer to a new symbol for the symbol set.
  ///
  /// This is a shortcut for sset.insert.
  ///
  void environment::insert(const symbol_ptr &i)
  {
    assert(i);

    sset.insert(i);
  }

  ///
  /// \param force_defined all the optional parameter have to be in a
  ///                      'well defined' state for the function to pass
  ///                      the test.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool environment::check(bool force_defined) const
  {
    if (force_defined && !code_length)
      return false;
    if (code_length && *code_length < 5)
      return false;

    if (force_defined && boost::indeterminate(elitism))
      return false;

    if (force_defined && !p_mutation)
      return false;
    if (p_mutation && (*p_mutation < 0.0 || *p_mutation > 1.0))
      return false;

    if (force_defined && !p_cross)
      return false;
    if (p_cross && (*p_cross < 0.0 || *p_cross > 1.0))
      return false;

    if (force_defined && !brood_recombination)
      return false;

    if (force_defined && boost::indeterminate(dss))
      return false;

    if (force_defined && !individuals)
      return false;
    if (individuals && *individuals <= 3)
      return false;

    if (force_defined && !par_tournament)
      return false;
    if (par_tournament)
    {
      if (*par_tournament == 0)
        return false;
      if (individuals && *par_tournament > *individuals)
        return false;
    }

    if (force_defined && !rep_tournament)
      return false;
    if (rep_tournament)
    {
      if (*rep_tournament == 0)
        return false;
      if (individuals && *rep_tournament > *individuals)
        return false;
    }

    if (force_defined && !mate_zone)
      return false;

    if (force_defined && !g_since_start)
      return false;

    return sset.check();
      // g_since_start > g_without_improvement;
  }
}  // Namespace vita
