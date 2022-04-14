/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/environment.h"

namespace vita
{
///
/// Class constructor.
///
/// \see environment::init
///
environment::environment()
{
}

///
/// Initialises the undefined parameters with "common" values.
///
/// \return a reference to the "filled" environment
///
/// Usually:
/// - the undefined parameters are tuned before the start of the search
///   (search::run calls search::tune_parameters) when there are enough data at
///   hand;
/// - the user doesn't have to fiddle with them (except after careful
///   consideration).
///
/// This function is mainly convenient for debugging purpose. The chosen values
/// are reasonable but most likely far from ideal.
///
/// \see search::tune_parameters
///
environment &environment::init()
{
  mep.code_length = 100;
  mep.patch_length = 1;
  elitism = trilean::yes;
  p_mutation = 0.04;
  p_cross = 0.9;
  brood_recombination = 1;
  dss = 1;
  layers = 1;
  individuals = 100;
  min_individuals = 2;
  tournament_size = 5;
  mate_zone = 20;
  generations = 100;
  max_stuck_time = std::numeric_limits<unsigned>::max();
  validation_percentage = 20;

  return *this;
}

///
/// Saves the environment in XML format.
///
/// \param[out] d output document for saving the environment
///
void environment::xml(tinyxml2::XMLDocument *d) const
{
  Expects(!stat.summary_file.empty());

  auto *root(d->RootElement());

  auto *e_environment(d->NewElement("environment"));
  root->InsertEndChild(e_environment);
  set_text(e_environment, "layers", layers);
  set_text(e_environment, "individuals", individuals);
  set_text(e_environment, "min_individuals", min_individuals);
  set_text(e_environment, "code_length", mep.code_length);
  set_text(e_environment, "patch_length", mep.patch_length);
  set_text(e_environment, "elitism", as_integer(elitism));
  set_text(e_environment, "mutation_rate", p_mutation);
  set_text(e_environment, "crossover_rate", p_cross);
  set_text(e_environment, "brood_recombination", brood_recombination);
  if (dss.has_value())
    set_text(e_environment, "dss", *dss);
  set_text(e_environment, "tournament_size", tournament_size);
  set_text(e_environment, "mating_zone", mate_zone);
  set_text(e_environment, "max_generations", generations);
  set_text(e_environment, "max_stuck_time", *max_stuck_time);
  if (validation_percentage.has_value())
    set_text(e_environment, "validation_percentage", *validation_percentage);
  set_text(e_environment, "cache_bits", cache_size);  // size `1u<<cache_size`

  auto *e_alps(d->NewElement("alps"));
  e_environment->InsertEndChild(e_alps);
  set_text(e_alps, "age_gap", alps.age_gap);
  set_text(e_alps, "p_same_layer", alps.p_same_layer);

  auto *e_team(d->NewElement("team"));
  e_environment->InsertEndChild(e_team);
  set_text(e_team, "individuals", team.individuals);

  auto *e_statistics(d->NewElement("statistics"));
  e_environment->InsertEndChild(e_statistics);
  set_text(e_statistics, "directory", stat.dir);
  set_text(e_statistics, "save_dynamics", stat.dynamic_file);
  set_text(e_statistics, "save_layers", stat.layers_file);
  set_text(e_statistics, "save_population", stat.population_file);
  set_text(e_statistics, "save_summary", stat.summary_file);
  set_text(e_statistics, "save_test", stat.test_file);
  set_text(e_statistics, "individual_format", stat.ind_format);

  auto *e_misc(d->NewElement("misc"));
  e_environment->InsertEndChild(e_misc);
  set_text(e_misc, "serialization_file", misc.serialization_file);
}

///
/// \param[in] force_defined all the undefined / auto-tuned parameters have to
///                          be in a "well defined" state for the function to
///                          pass the test
/// \return                  `true` if the object passes the internal
///                          consistency check
///
bool environment::is_valid(bool force_defined) const
{
  if (force_defined)
  {
    if (!mep.code_length)
    {
      vitaERROR << "Undefined code_length data member";
      return false;
    }

    if (!mep.patch_length)
    {
      vitaERROR << "Undefined patch_length data member";
      return false;
    }

    if (elitism == trilean::unknown)
    {
      vitaERROR << "Undefined elitism data member";
      return false;
    }

    if (p_mutation < 0.0)
    {
      vitaERROR << "Undefined p_mutation data member";
      return false;
    }

    if (p_cross < 0.0)
    {
      vitaERROR << "Undefined p_cross data member";
      return false;
    }

    if (!brood_recombination)
    {
      vitaERROR << "Undefined brood_recombination data member";
      return false;
    }

    if (!layers)
    {
      vitaERROR << "Undefined layer data member";
      return false;
    }

    if (!individuals)
    {
      vitaERROR << "Undefined `individuals` data member";
      return false;
    }

    if (!min_individuals)
    {
      vitaERROR << "Undefined `min_individuals` data member";
      return false;
    }

    if (!tournament_size)
    {
      vitaERROR << "Undefined tournament_size data member";
      return false;
    }

    if (!mate_zone)
    {
      vitaERROR << "Undefined mate_zone data member";
      return false;
    }

    if (!generations)
    {
      vitaERROR << "Undefined generations data member";
      return false;
    }

    if (!max_stuck_time.has_value())
    {
      vitaERROR << "Undefined max_stuck_time data member";
      return false;
    }

    if (!alps.age_gap)
    {
      vitaERROR << "Undefined age_gap parameter";
      return false;
    }

    if (alps.p_same_layer < 0.0)
    {
      vitaERROR << "Undefined p_same_layer parameter";
      return false;
    }

    if (!team.individuals)
    {
      vitaERROR << "Undefined team size parameter";
      return false;
    }
  }  // if (force_defined)

  if (mep.code_length == 1)
  {
    vitaERROR << "`code_length` is too short";
    return false;
  }

  if (mep.code_length && mep.patch_length
      && mep.patch_length >= mep.code_length)
  {
    vitaERROR << "`patch_length` must be shorter than `code_length`";
    return false;
  }

  if (p_mutation > 1.0)
  {
    vitaERROR << "`p_mutation` out of range";
    return false;
  }

  if (p_cross > 1.0)
  {
    vitaERROR << "`p_cross` out of range";
    return false;
  }

  if (validation_percentage.has_value() && *validation_percentage >= 100)
  {
    vitaERROR << "`validation_percentage` out of range";
    return false;
  }

  if (dss.has_value() && *dss == 0)
  {
    vitaERROR << "`dss` out of range";
    return false;
  }

  if (alps.p_same_layer > 1.0)
  {
    vitaERROR << "`p_same_layer` out of range";
    return false;
  }

  if (min_individuals == 1)
  {
    vitaERROR << "At least 2 individuals for layer";
    return false;
  }

  if (individuals && min_individuals && individuals < min_individuals)
  {
    vitaERROR << "Too few individuals";
    return false;
  }

  if (individuals && tournament_size && tournament_size > individuals)
  {
    vitaERROR << "`tournament_size` (" << tournament_size
              << ") cannot be greater than population size ("
              << individuals << ")";
    return false;
  }

  if (mate_zone && tournament_size && tournament_size > mate_zone)
  {
    vitaERROR << "`tournament_size` (" << tournament_size
              << ") cannot be greater than `mate_zone` (" << mate_zone
              << ")";
    return false;
  }

  if (stat.dir.has_filename())
  {
    vitaERROR << "`stat.dir` must contain a directory, not a file ("
              << stat.dir << ")";
    return false;
  }

  if (!stat.dynamic_file.empty() && !stat.dynamic_file.has_filename())
  {
    vitaERROR << "`stat.dynamic_file` must specify a file ("
              << stat.dynamic_file << ")";
    return false;
  }

  if (!stat.layers_file.empty() && !stat.layers_file.has_filename())
  {
    vitaERROR << "`stat.layers_file` must specify a file ("
              << stat.layers_file << ")";
    return false;
  }

  if (!stat.population_file.empty() && !stat.population_file.has_filename())
  {
    vitaERROR << "`stat.population_file` must specify a file ("
              << stat.population_file << ")";
    return false;
  }

  if (!stat.summary_file.empty() && !stat.summary_file.has_filename())
  {
    vitaERROR << "`stat.summary_file` must specify a file ("
              << stat.summary_file << ")";
    return false;
  }

  if (!stat.test_file.empty() && !stat.test_file.has_filename())
  {
    vitaERROR << "`stat.test_file` must specify a file ("
              << stat.test_file << ")";
    return false;
  }

  return true;
}

}  // namespace vita
