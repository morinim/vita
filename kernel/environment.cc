/**
 *
 *  \file environment.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
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
  /// \param[in] initialize if \c true initialize every parameter so that
  ///                       \c check(true) passes the test.
  /// Class constructor. Default values are quite standard, but specific
  /// problems need ad-hoc tuning.
  ///
  environment::environment(bool initialize)
    : elitism(boost::indeterminate),
      dss(boost::indeterminate),
      arl(boost::indeterminate),
      ttable_size(16),
      stat_dir(""),
      stat_arl(false), stat_dynamic(false), stat_summary(false)
  {
    if (initialize)
    {
      code_length = 100;
      elitism = true;
      p_mutation = 0.1;
      p_cross = 0.7;
      brood_recombination = 0;
      dss = true;
      individuals = 100;
      par_tournament = 2;
      rep_tournament = 4;
      mate_zone = 20;
      g_since_start = 100;
      g_without_improvement = 0;
      arl = false;

      assert(check(true, true));
    }
    else
      assert(check(true, false));
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
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \param force_defined all the optional parameter have to be in a
  ///                      'well defined' state for the function to pass
  ///                      the test.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool environment::check(bool verbose, bool force_defined) const
  {
    if (force_defined && !code_length)
    {
      if (verbose)
        std::cerr << "Undefined code_length data member." << std::endl;
      return false;
    }
    if (code_length && *code_length < 1)
    {
      if (verbose)
        std::cerr << "code_length is too short." << std::endl;
      return false;
    }

    if (force_defined && boost::indeterminate(elitism))
    {
      if (verbose)
        std::cerr << "Undefined elitism data member." << std::endl;
      return false;
    }

    if (force_defined && !p_mutation)
    {
      if (verbose)
        std::cerr << "Undefined p_mutation data member." << std::endl;
      return false;
    }
    if (p_mutation && (*p_mutation < 0.0 || *p_mutation > 1.0))
    {
      if (verbose)
        std::cerr << "p_mutation out of range." << std::endl;
      return false;
    }

    if (force_defined && !p_cross)
    {
      if (verbose)
        std::cerr << "Undefined p_cross data member." << std::endl;
      return false;
    }
    if (p_cross && (*p_cross < 0.0 || *p_cross > 1.0))
    {
      if (verbose)
        std::cerr << "p_cross out of range." << std::endl;
      return false;
    }

    if (force_defined && !brood_recombination)
    {
      if (verbose)
        std::cerr << "Undefined brood_recombination data member." << std::endl;
      return false;
    }

    if (force_defined && boost::indeterminate(dss))
    {
      if (verbose)
        std::cerr << "Undefined dss data member." << std::endl;
      return false;
    }

    if (force_defined && !individuals)
    {
      if (verbose)
        std::cerr << "Undefined individuals data member." << std::endl;
      return false;
    }
    if (individuals && *individuals <= 3)
    {
      if (verbose)
        std::cerr << "Too few individuals." << std::endl;
      return false;
    }

    if (force_defined && !par_tournament)
    {
      if (verbose)
        std::cerr << "Undefined par_tournament data member." << std::endl;
      return false;
    }
    if (par_tournament)
    {
      if (*par_tournament == 0)
      {
        if (verbose)
          std::cerr << "par_tournament must be greater than 0." << std::endl;
        return false;
      }
      if (individuals && *par_tournament > *individuals)
      {
        if (verbose)
          std::cerr << "par_tournament cannot be greater than individuals."
                    << std::endl;
        return false;
      }
    }

    if (force_defined && !rep_tournament)
    {
      if (verbose)
        std::cerr << "Undefined rep_tournament data member." << std::endl;
      return false;
    }
    if (rep_tournament)
    {
      if (*rep_tournament == 0)
      {
        if (verbose)
          std::cerr << "rep_tournament must be greater than 0." << std::endl;
        return false;
      }
      if (individuals && *rep_tournament > *individuals)
      {
        if (verbose)
          std::cerr << "rep_tournament cannot be greater than individuals."
                    << std::endl;
        return false;
      }
    }

    if (force_defined && !mate_zone)
    {
      if (verbose)
        std::cerr << "Undefined mate_zone data member." << std::endl;
      return false;
    }

    if (force_defined && !g_since_start)
    {
      if (verbose)
        std::cerr << "Undefined g_since_start data member." << std::endl;
      return false;
    }

    if (force_defined && !g_without_improvement)
    {
      if (verbose)
        std::cerr << "Undefined g_without_improvement data member."
                  << std::endl;
      return false;
    }

    if (force_defined && boost::indeterminate(arl))
    {
      if (verbose)
        std::cerr << "Undefined arl data member." << std::endl;
      return false;
    }

    return sset.check();
  }
}  // Namespace vita
