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
  const char environment::arl_filename[] =         "arl";
  const char environment::dyn_filename[] =     "dynamic";
  const char environment::sum_filename[] =     "summary";

  ///
  /// Class constructor. Default values are quite reasonable, but specific
  /// problems need ad-hoc parameters.
  ///
  environment::environment()
    : code_length(100),
      elitism(true),
      p_mutation(2.0/code_length),
      p_cross(0.7),
      individuals(100),
      par_tournament(2),
      rep_tournament(unsigned(std::log(static_cast<double>(individuals)))),
      mate_zone(9),
      g_since_start(100), g_without_improvement(0),
      arl(true),
      force_input(true),
      ttable_size(16),
      stat_dir(""),
      stat_arl(false), stat_dynamic(false), stat_summary(false)
  {
    assert(check());
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

    const std::string env(path+"environment.");
    pt->put(env+"population_size", individuals);
    pt->put(env+"max_program_length", code_length);
    pt->put(env+"elitism", elitism);
    pt->put(env+"mutation_rate", p_mutation);
    pt->put(env+"crossover_rate", p_cross);
    pt->put(env+"parent_tournament_size", par_tournament);
    pt->put(env+"replacement_tournament_size", rep_tournament);
    pt->put(env+"mating_zone", mate_zone);
    pt->put(env+"max_gens_since_start", g_since_start);
    pt->put(env+"max_gens_wo_imp", g_without_improvement);
    pt->put(env+"arl", arl);
    pt->put(env+"force_input", force_input);
    pt->put(env+"ttable_bits", ttable_size);  // size 1u << ttable_size.
    pt->put(env+"statistics.directory", stat_dir);
    pt->put(env+"statistics.save_arl", stat_arl);
    pt->put(env+"statistics.save_dynamics", stat_dynamic);
    pt->put(env+"statistics.save_summary", stat_summary);
  }

  ///
  /// \param[in] i pointer to a new symbol for the symbol set.
  ///
  void environment::insert(symbol_ptr i)
  {
    const bool special(force_input && i->terminal() &&
                       static_cast<terminal *>(i.get())->input());
    sset.insert(i, special);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool environment::check() const
  {
    return
      code_length &&

      0 <= p_mutation && 0 <= p_cross && 0 < p_mutation+p_cross &&

      3 < individuals &&

      0 < par_tournament && par_tournament <= individuals &&
      0 < rep_tournament && rep_tournament <= individuals &&

      sset.check();

      // g_since_start > g_without_improvement;
  }
}  // Namespace vita
