/**
 *
 *  \file environment.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <iostream>

#include <boost/logic/tribool_io.hpp>

#include "environment.h"
#include "terminal.h"

namespace vita
{
  const char environment::arl_filename[] =        "arl";
  const char environment::dyn_filename[] =    "dynamic";
  const char environment::pop_filename[] = "population";
  const char environment::sum_filename[] =    "summary";
  const char environment::tst_filename[] =       "test";

  ///
  /// \param[in] initialize if \c true initializes every parameter in such a
  ///                       way as to allow the object to pass
  ///                       environment::debug().
  ///
  /// Class constructor. Default values are quite standard, but specific
  /// problems need ad-hoc tuning.
  ///
  /// \see search::tune_parameters
  ///
  environment::environment(bool initialize)
    : verbosity(2),
      code_length(0),
      patch_length(0),
      layers(0),
      age_gap(0),
      individuals(0),
      elitism(boost::indeterminate),
      tournament_size(0),
      dss(boost::indeterminate),
      arl(boost::indeterminate),
      ttable_size(16),
      stat_dir(""),
      stat_arl(false), stat_dynamic(false), stat_population(false),
      stat_summary(false),
      a_threashold(-1.0)
  {
    if (initialize)
    {
      code_length = 100;
      patch_length = 1;
      elitism = true;
      p_mutation = 0.04;
      p_cross = 0.9;
      brood_recombination = 0;
      dss = true;
      layers = 1;
      age_gap = 20;
      individuals = 100;
      tournament_size = 5;
      mate_zone = 20;
      g_since_start = 100;
      g_without_improvement = 0;
      arl = false;
      validation_ratio = 0.2;

      assert(debug(true, true));
    }
    else
      assert(debug(true, false));
  }

  ///
  /// \param[out] pt output tree.
  /// \param[in] path \a path where to store the environment data file.
  ///
  /// Saves the environment using the \a pt boost property tree.
  ///
  void environment::log(boost::property_tree::ptree *const pt,
                        const std::string &path) const
  {
    assert(stat_summary);

    const std::string env(path + "environment.");
    pt->put(env + "layers", layers);
    pt->put(env + "individuals", individuals);
    pt->put(env + "code_length", code_length);
    pt->put(env + "patch_length", patch_length);
    pt->put(env + "elitism", elitism);
    pt->put(env + "mutation_rate", p_mutation);
    pt->put(env + "crossover_rate", p_cross);
    pt->put(env + "brood_recombination", brood_recombination);
    pt->put(env + "dss", dss);
    pt->put(env + "tournament_size", tournament_size);
    pt->put(env + "mating_zone", mate_zone);
    pt->put(env + "max_gens_since_start", g_since_start);
    pt->put(env + "max_gens_wo_imp", g_without_improvement);
    pt->put(env + "arl", arl);
    pt->put(env + "validation_ratio", validation_ratio);
    pt->put(env + "ttable_bits", ttable_size);  // size 1u << ttable_size.
    pt->put(env + "statistics.directory", stat_dir);
    pt->put(env + "statistics.save_arl", stat_arl);
    pt->put(env + "statistics.save_dynamics", stat_dynamic);
    pt->put(env + "statistics.save_population", stat_population);
    pt->put(env + "statistics.save_summary", stat_summary);
  }

  ///
  /// \param[in] i pointer to a new symbol for the symbol set.
  ///
  /// This is a shortcut for symbol_set::insert.
  ///
  void environment::insert(const symbol::ptr &i)
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
  bool environment::debug(bool verbose, bool force_defined) const
  {
    if (force_defined)
    {
      if (!code_length)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined code_length data member"
                    << std::endl;
        return false;
      }

      if (!patch_length)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined patch_length data member"
                    << std::endl;
        return false;
      }

      if (boost::indeterminate(elitism))
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined elitism data member"
                    << std::endl;
        return false;
      }

      if (!p_mutation)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined p_mutation data member"
                    << std::endl;
        return false;
      }

      if (!p_cross)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined p_cross data member"
                    << std::endl;
        return false;
      }

      if (!brood_recombination)
      {
        if (verbose)
          std::cerr << k_s_debug
                    << " Undefined brood_recombination data member"
                    << std::endl;
        return false;
      }

      if (boost::indeterminate(dss))
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined dss data member" << std::endl;
        return false;
      }

      if (!layers)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined layers data member"
                    << std::endl;
        return false;
      }

      if (!individuals)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined individuals data member"
                    << std::endl;
        return false;
      }

      if (!tournament_size)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined tournament_size data member"
                    << std::endl;
        return false;
      }

      if (!mate_zone)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined mate_zone data member"
                    << std::endl;
        return false;
      }

      if (!g_since_start)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined g_since_start data member"
                    << std::endl;
        return false;
      }

      if (!g_without_improvement)
      {
        if (verbose)
          std::cerr << k_s_debug
                    << " Undefined g_without_improvement data member"
                    << std::endl;
        return false;
      }

      if (boost::indeterminate(arl))
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined arl data member" << std::endl;
        return false;
      }

      if (!validation_ratio)
      {
        if (verbose)
          std::cerr << k_s_debug << " Undefined validation_ratio data member"
                    << std::endl;
        return false;
      }
    }  // if (force_defined)

    if (code_length == 1)
    {
      if (verbose)
        std::cerr << k_s_debug << " code_length is too short" << std::endl;
      return false;
    }

    if (code_length && patch_length && patch_length >= code_length)
    {
      std::cerr << k_s_debug
                << " patch_length must be shorter than code_length"
                << std::endl;
      return false;
    }

    if (p_mutation && (*p_mutation < 0.0 || *p_mutation > 1.0))
    {
      if (verbose)
        std::cerr << k_s_debug << " p_mutation out of range" << std::endl;
      return false;
    }

    if (p_cross && (*p_cross < 0.0 || *p_cross > 1.0))
    {
      if (verbose)
        std::cerr << k_s_debug << " p_cross out of range" << std::endl;
      return false;
    }

    if (layers && individuals && individuals < 2 * layers)
    {
      if (verbose)
        std::cerr << k_s_debug << " individuals / layers ratio ("
                  << individuals << '/' << layers << ") is too low"
                  << std::endl;
      return false;
    }

    if (individuals && individuals <= 3)
    {
      if (verbose)
        std::cerr << k_s_debug << " Too few individuals" << std::endl;
      return false;
    }

    if (individuals && tournament_size && layers &&
        tournament_size > individuals / layers)
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " tournament_size (" << tournament_size
                  << ") cannot be greater than individuals in a layer ("
                  << individuals / layers << ')' << std::endl;
      return false;
    }

    if (mate_zone && tournament_size && tournament_size > *mate_zone)
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " tournament_size (" << tournament_size
                  << ") cannot be greater than mate_zone (" << *mate_zone
                  << ')' << std::endl;
      return false;
    }

    return sset.debug();
  }
}  // Namespace vita
