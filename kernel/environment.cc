/**
 *
 *  \file environment.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */
  
#include "environment.h"

namespace vita
{
 
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
  /// \param[out] s output stream.
  ///
  /// Prints out the informations about the environment using the \a s output
  /// stream.
  void
  environment::log(std::ostream &s) const
  {
    s << "Population size: " << individuals << std::endl

      << "Maximum program length:  " << code_length << std::endl

      << "Mutation rate: " << p_mutation << " (" << p_mutation*code_length
      << " per program)" << std::endl

      << "Crossover rate: " << p_cross << std::endl

      << "Parents' tournament size: " << par_tournament << std::endl

      << "Replacements' tournament size: " << rep_tournament << std::endl

      << "Mating zone: " << mate_zone << std::endl

      << "Maximum number of generations since start: " << g_since_start 
      << std::endl

      << "Maximum number of generations without improvements: " 
      << g_without_improvement << std::endl

      << "ARL: " << arl << std::endl

      << "TTable size: " << (1u << ttable_size) << " (" << ttable_size 
      << " bits)" << std::endl

      << "Statistics directory: " << stat_dir << std::endl

      << "Statistics period: " << stat_period << std::endl

      << "List of ADF: " << stat_arl << std::endl

      << "Dynamic execution file: " << stat_dynamic << std::endl;
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
