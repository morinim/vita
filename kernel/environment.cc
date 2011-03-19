/**
 *
 *  \file environment.cc
 *
 *  \author Manlio Morini
 *  \date 2011/03/15
 *
 *  This file is part of VITA
 *
 */
  
#include <iostream>

#include "environment.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace vita
{
 
  const char environment::arl_filename[] =         "arl";
  const char environment::dyn_filename[] =     "dynamic";
  const char environment::env_filename[] = "environment";
  const char environment::sum_filename[] =     "summary";

  ///
  /// Class constructor. Default values are quite reasonable, but specific
  /// problems need ad-hoc parameters.
  ///
  environment::environment()
    : code_length(100),
      p_mutation(2.0/code_length),
      p_cross(0.7),
      individuals(100),
      par_tournament(2), 
      rep_tournament(unsigned(std::log(double(individuals)))),
      mate_zone(9),
      g_since_start(100), g_without_improvement(0),
      arl(true),
      ttable_size(16),
      stat_arl(false),
      stat_dir(""), stat_period(0), 
      stat_env(false), stat_dynamic(false), stat_summary(false)
  {
    assert(check());
  }

  ///
  /// \return true if the operation succeed.
  ///
  /// Saves the informations regarding the environment in a new file
  /// (name: env_filename, folder: stat_dir).
  ///
  bool
  environment::log() const
  {
    assert(stat_env);

    const std::string filename(stat_dir + "/" + env_filename);
    std::ofstream logs(filename.c_str());

    if (logs.good()) 
      log(logs);

    return logs.good();
  }

  ///
  /// \param[out] s output stream.
  ///
  /// Saves the informations regarding the environment using the \a s output
  /// stream.
  ///
  void
  environment::log(std::ostream &s) const
  {
    boost::property_tree::ptree pt;

    const std::string env("environment.");

    pt.put(env+"population_size",individuals);
    pt.put(env+"max_program_length",code_length);
    pt.put(env+"mutation_rate",p_mutation);
    pt.put(env+"crossover_rate",p_cross);
    pt.put(env+"parent_tournament_size",par_tournament);
    pt.put(env+"replacement_tournament_size",rep_tournament);
    pt.put(env+"mating_zone",mate_zone);
    pt.put(env+"max_gens_since_start",g_since_start);
    pt.put(env+"max_gens_wo_imp",g_without_improvement);
    pt.put(env+"arl",arl);
    pt.put(env+"ttable_bits",ttable_size); // size 1u << ttable_size.
    pt.put(env+"statistics.directory",stat_dir);
    pt.put(env+"statistics.period",stat_period);
    pt.put(env+"statistics.save_arl",stat_arl);
    pt.put(env+"statistics.save_dynamics",stat_dynamic);
    pt.put(env+"statistics.save_env",stat_env);
    pt.put(env+"statistics.save_summary",stat_summary);

    using namespace boost::property_tree::xml_parser;
    write_xml(s,pt,xml_writer_make_settings(' ',2));
  }

  ///
  /// \param[in] i pointer to a new symbol for the symbol set.
  ///
  void
  environment::insert(symbol *const i)
  {
    sset.insert(i);
  }

  ///
  /// \return true if the individual passes the internal consistency check.
  ///
  bool
  environment::check() const
  {
    return
      code_length &&

      0 <= p_mutation && 0 <= p_cross && 0 < p_mutation+p_cross &&

      3 < individuals &&

      0 < par_tournament && par_tournament <= individuals &&
      0 < rep_tournament && rep_tournament <= individuals &&

      sset.check();

      //g_since_start > g_without_improvement;
  }  

}  // Namespace vita
