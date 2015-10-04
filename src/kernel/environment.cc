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
///
/// \param[in] ss a pointer to the symbol set used in the current environment.
/// \param[in] initialize if `true` initializes every parameter in such a
///                       way as to allow the object to pass
///                       `environment::debug(..., true)`.
///
/// Class constructor. Default values are quite standard, but specific
/// problems need ad-hoc tuning.
///
/// \see search::tune_parameters
///
environment::environment(symbol_set *ss, bool initialize) : sset(ss)
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
    layers = 1;
    individuals = 100;
    tournament_size = 5;
    mate_zone = 20;
    generations = 100;
    g_without_improvement =
      std::numeric_limits<decltype(g_without_improvement)>::max();
    arl = trilean::no;
    validation_percentage = 20;
  }

  assert(debug(initialize));
}

///
/// \param[out] p output document where to save the environment.
///
/// Saves the environment (XML format).
///
void environment::xml(tinyxml2::XMLPrinter *p) const
{
  assert(stat.summary);

  auto tti =
  [](trilean v)
  {
    return static_cast<std::underlying_type<trilean>::type>(v);
  };

  // TO BE CHANGED WITH POLIMORPHIC LAMBDA when switching to C++14
  // auto print = [&](const std::string &s, auto v) { ... }
#define env_print(s, v) p->OpenElement(s); p->PushText(v); p->CloseElement()

  p->OpenElement("environment");

  env_print("layers", layers);
  env_print("individuals", individuals);
  env_print("code_length", code_length);
  env_print("patch_length", patch_length);
  env_print("elitism", tti(elitism));
  env_print("mutation_rate", p_mutation);
  env_print("crossover_rate", p_cross);
  env_print("brood_recombination", *brood_recombination);
  env_print("dss", tti(dss));
  env_print("tournament_size", tournament_size);
  env_print("mating_zone", *mate_zone);
  env_print("max_generations", generations);
  env_print("max_gens_wo_imp", g_without_improvement);
  env_print("arl", tti(arl));

  p->OpenElement("alps");
  env_print("age_gap", alps.age_gap);
  env_print("p_same_layer", alps.p_same_layer);
  p->CloseElement();

  p->OpenElement("team");
  env_print("individuals", team.individuals);
  p->CloseElement();

  env_print("validation_percentage", validation_percentage);
  env_print("ttable_bits", ttable_size);  // size 1u << ttable_size.

  p->OpenElement("statistics");
  env_print("directory", stat.dir.c_str());
  env_print("save_arl", stat.arl);
  env_print("save_dynamics", stat.dynamic);
  env_print("save_layers", stat.layers);
  env_print("save_population", stat.population);
  env_print("save_summary", stat.summary);
  p->CloseElement();

  p->CloseElement();  // </environment>

#undef env_save_print
}

///
/// \param force_defined all the optional parameter have to be in a
///                      'well defined' state for the function to pass
///                      the test.
/// \return `true` if the object passes the internal consistency check.
///
bool environment::debug(bool force_defined) const
{
  if (force_defined)
  {
    if (!code_length)
    {
      print.error("Undefined code_length data member");
      return false;
    }

    if (!patch_length)
    {
      print.error("Undefined patch_length data member");
      return false;
    }

    if (elitism == trilean::unknown)
    {
      print.error("Undefined elitism data member");
      return false;
    }

    if (p_mutation < 0.0)
    {
      print.error("Undefined p_mutation data member");
      return false;
    }

    if (p_cross < 0.0)
    {
      print.error("Undefined p_cross data member");
      return false;
    }

    if (!brood_recombination)
    {
      print.error("Undefined brood_recombination data member");
      return false;
    }

    if (dss == trilean::unknown)
    {
      print.error("Undefined dss data member");
      return false;
    }

    if (!layers)
    {
      print.error("Undefined layer data member");
      return false;
    }

    if (!individuals)
    {
      print.error("Undefined individuals data member");
      return false;
    }

    if (!tournament_size)
    {
      print.error("Undefined tournament_size data member");
      return false;
    }

    if (!mate_zone)
    {
      print.error("Undefined mate_zone data member");
      return false;
    }

    if (!generations)
    {
      print.error("Undefined generations data member");
      return false;
    }

    if (!g_without_improvement)
    {
      print.error("Undefined g_without_improvement data member");
      return false;
    }

    if (arl == trilean::unknown)
    {
      print.error("Undefined arl data member");
      return false;
    }

    if (validation_percentage > 100)
    {
      print.error("Undefined validation_percentage data member");
      return false;
    }

    if (!alps.age_gap)
    {
      print.error("Undefined age_gap parameter");
      return false;
    }

    if (alps.p_same_layer < 0.0)
    {
      print.error("Undefined p_same_layer parameter");
      return false;
    }

    if (!team.individuals)
    {
      print.error("Undefined team size parameter");
      return false;
    }
  }  // if (force_defined)

  if (code_length == 1)
  {
    print.error("code_length is too short");
    return false;
  }

  if (code_length && patch_length && patch_length >= code_length)
  {
    print.error("patch_length must be shorter than code_length");
    return false;
  }

  if (p_mutation > 1.0)
  {
    print.error("p_mutation out of range");
    return false;
  }

  if (p_cross > 1.0)
  {
    print.error("p_cross out of range");
    return false;
  }

  if (alps.p_same_layer > 1.0)
  {
    print.error("p_same_layer out of range");
    return false;
  }

  if (individuals && individuals <= 3)
  {
    print.error("Too few individuals");
    return false;
  }

  if (individuals && tournament_size && tournament_size > individuals)
  {
    print.error("tournament_size (", tournament_size,
                ") cannot be greater than population size (", individuals,
                ")");
    return false;
  }

  if (mate_zone && tournament_size && tournament_size > *mate_zone)
  {
    print.error("tournament_size (", tournament_size,
                ") cannot be greater than mate_zone (", *mate_zone, ")");
    return false;
  }

  return true;
}
}  // namespace vita
