/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <iostream>
#include <limits>

#include "kernel/environment.h"

namespace vita
{
const char environment::arl_filename[] =        "arl";
const char environment::dyn_filename[] =    "dynamic";
const char environment::lys_filename[] =     "layers";
const char environment::pop_filename[] = "population";
const char environment::sum_filename[] =    "summary";
const char environment::tst_filename[] =       "test";

///
/// \param[in] initialize if `true` initializes every parameter in such a
///                       way as to allow the object to pass
///                       environment::debug().
///
/// Class constructor. Default values are quite standard, but specific
/// problems need ad-hoc tuning.
///
/// \see search::tune_parameters
///
environment::environment(bool initialize)
{
  if (initialize)
  {
    code_length = 100;
    patch_length = 1;
    elitism = trilean::yes;
    p_mutation = 0.04;
    p_cross = 0.9;
    brood_recombination = 0;
    dss = trilean::yes;
    layers = 4;
    individuals = 100;
    tournament_size = 5;
    mate_zone = 20;
    generations = 100;
    g_without_improvement =
      std::numeric_limits<decltype(g_without_improvement)>::max();
    arl = trilean::no;
    validation_percentage = 20;

    assert(debug(true, true));
  }
  else
    assert(debug(true, false));
}

///
/// \param[out] pt output tree.
/// \param[in] path where to store the environment data file.
///
/// Saves the environment using the `pt` boost property tree.
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
  pt->put(env + "max_generations", generations);
  pt->put(env + "max_gens_wo_imp", g_without_improvement);
  pt->put(env + "arl", arl);
  pt->put(env + "alps.age_gap", alps.age_gap);
  pt->put(env + "alps.p_same_layer", alps.p_same_layer);
  pt->put(env + "team.individuals", team.individuals);
  pt->put(env + "validation_percentage", validation_percentage);
  pt->put(env + "ttable_bits", ttable_size);  // size 1u << ttable_size.
  pt->put(env + "statistics.directory", stat_dir);
  pt->put(env + "statistics.save_arl", stat_arl);
  pt->put(env + "statistics.save_dynamics", stat_dynamic);
  pt->put(env + "statistics.save_layers", stat_layers);
  pt->put(env + "statistics.save_population", stat_population);
  pt->put(env + "statistics.save_summary", stat_summary);
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \param force_defined all the optional parameter have to be in a
///                      'well defined' state for the function to pass
///                      the test.
/// \return `true` if the object passes the internal consistency check.
///
bool environment::debug(bool verbose, bool force_defined) const
{
  if (force_defined)
  {
    if (!code_length)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined code_length data member\n";
      return false;
    }

    if (!patch_length)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined patch_length data member\n";
      return false;
    }

    if (elitism == trilean::unknown)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined elitism data member\n";
      return false;
    }

    if (p_mutation < 0.0)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined p_mutation data member\n";
      return false;
    }

    if (p_cross < 0.0)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined p_cross data member\n";
      return false;
    }

    if (!brood_recombination)
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " Undefined brood_recombination data member\n";
      return false;
    }

    if (dss == trilean::unknown)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined dss data member\n";
      return false;
    }

    if (!layers)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined layer data member\n";
      return false;
    }

    if (!individuals)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined individuals data member\n";
      return false;
    }

    if (!tournament_size)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined tournament_size data member\n";
      return false;
    }

    if (!mate_zone)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined mate_zone data member\n";
      return false;
    }

    if (!generations)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined generations data member\n";
      return false;
    }

    if (!g_without_improvement)
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " Undefined g_without_improvement data member\n";
      return false;
    }

    if (arl == trilean::unknown)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined arl data member\n";
      return false;
    }

    if (validation_percentage > 100)
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " Undefined validation_percentage data member\n";
      return false;
    }

    if (!alps.age_gap)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined age_gap parameter\n";
      return false;
    }

    if (alps.p_same_layer < 0.0)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined p_same_layer parameter\n";
      return false;
    }

    if (!team.individuals)
    {
      if (verbose)
        std::cerr << k_s_debug << " Undefined team size parameter\n";

      return false;
    }
  }  // if (force_defined)

  if (code_length == 1)
  {
    if (verbose)
      std::cerr << k_s_debug << " code_length is too short\n";
    return false;
  }

  if (code_length && patch_length && patch_length >= code_length)
  {
    std::cerr << k_s_debug
              << " patch_length must be shorter than code_length\n";
    return false;
  }

  if (p_mutation > 1.0)
  {
    if (verbose)
      std::cerr << k_s_debug << " p_mutation out of range\n";
    return false;
  }

  if (p_cross > 1.0)
  {
    if (verbose)
      std::cerr << k_s_debug << " p_cross out of range\n";
    return false;
  }

  if (alps.p_same_layer > 1.0)
  {
    if (verbose)
      std::cerr << k_s_debug << " p_same_layer out of range\n";
    return false;
  }

  if (individuals && individuals <= 3)
  {
    if (verbose)
      std::cerr << k_s_debug << " Too few individuals\n";
    return false;
  }

  if (individuals && tournament_size && tournament_size > individuals)
  {
    if (verbose)
      std::cerr << k_s_debug << " tournament_size (" << tournament_size
                << ") cannot be greater than population size ("
                << individuals << ")\n";
    return false;
  }

  if (mate_zone && tournament_size && tournament_size > *mate_zone)
  {
    if (verbose)
      std::cerr << k_s_debug << " tournament_size (" << tournament_size
                << ") cannot be greater than mate_zone (" << *mate_zone
                << ")\n";
    return false;
  }

  return true;
}
}  // namespace vita
