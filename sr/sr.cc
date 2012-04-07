/**
 *
 *  \file sr.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
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

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "kernel/environment.h"
#include "kernel/random.h"
#include "kernel/search.h"
#include "kernel/src_problem.h"
#include "kernel/primitive/factory.h"

const std::string vita_sr_version1(
  "Vita - Symbolic Regression and classification v0.9.1"
);
const std::string vita_sr_version2(
  "Copyright (c) 2011 EOS Development (http://www.eosdev.it)"
);

unsigned verbose(true);

unsigned runs(1);


vita::src_problem problem(-5.0);

///
/// \param[in] s
/// \return
///
bool is_true(const std::string &s)
{
  return s != "0" && !boost::iequals(s, "false");
}

///
/// \param[in] argc
/// \param[in] argv
/// \return
///
bool parse_command_line(int argc, char *argv[])
{
  const std::string k_arl("arl");
  const std::string k_brood("brood");
  const std::string k_code_length("code-length");
  const std::string k_code_length_l(k_code_length + ",l");
  const std::string k_crossover_rate("crossover-rate");
  const std::string k_crossover_rate_c(k_crossover_rate + ",c");
  const std::string k_dss("dss");
  const std::string k_elitism("elitism");
  const std::string k_g_since_start("g-since-start");
  const std::string k_g_since_start_g(k_g_since_start + ",g");
  const std::string k_gwi("gwi");
  const std::string k_mate_zone("mate-zone");
  const std::string k_mutation_rate("mutation-rate");
  const std::string k_parent_tournament("parent-tournament");
  const std::string k_population_size("population-size");
  const std::string k_population_size_P(k_population_size + ",P");

  unsigned random_seed;
  std::string data_file, f_f, symbol_file;

  try
  {
    // Declare a group of options that will be allowed only on command line.
    po::options_description generic("Generic options");
    generic.add_options()
      ("version,v", "print version string.")
      ("help,h", "produces help message.")
      ("verbose", po::value<unsigned>(&verbose)->implicit_value(1),
       "verbosity (optionally specify level, 0 == quiet).");

    po::options_description data("Data");
    data.add_options()
      ("data,d", po::value(&data_file), "Data file.")
      ("symbols,s", po::value(&symbol_file), "Symbol file.");

    po::options_description config("Config");
    config.add_options()
      ("ttable",
       po::value(&problem.env.ttable_size)->default_value(
         problem.env.ttable_size),
       "Number of bits used for the ttable (ttable contains 2^bit elements).")
      ("random-seed",
       po::value(&random_seed),
       "Sets the seed for the pseudo-random number generator. "\
       "Pseudo-random sequences are repeatable by using the same seed value.");

    // Declare a group of options that will be allowed both on command line
    // and in config file.
    po::options_description individual("Individual");
    individual.add_options()
      (k_code_length_l.c_str(), po::value<unsigned>(),
       "Sets the code/genome length of an individual");

    // ------> About boost::optional and boost::program_options <------
    // To date there isn't a direct approach:
    //
    //   optional<double> x;
    //   desc.add_options()("x", po::value(&x));
    //
    // is NOT supported, while this works:
    //
    //   desc.add_options()("x", po::value<double>()->notifier(var(x) = _1));
    //
    // but it is quite involuted and I prefer a longer but simpler approach.
    // See also http://lists.boost.org/Archives/boost/2004/06/66572.php
    // ----------------------------------------------------------------

    // Declare a group of options that will be allowed both on command line
    // and in config file.
    po::options_description evolution("Evolution");
    evolution.add_options()
      (k_population_size_P.c_str(), po::value<unsigned>(),
       "Sets the number of programs/individuals in the population.")
      (k_elitism.c_str(), po::value<std::string>(),
       "When elitism is true an individual will never replace a better one.")
      (k_mutation_rate.c_str(), po::value<double>(),
       "Sets the overall probability of mutation of the individuals that have "\
       "been selected as winners in a tournament. Range is [0,1].")
      (k_crossover_rate_c.c_str(), po::value<double>(),
       "Sets the overall probability that crossover will occour between two "\
       "winners in a tournament. Range is [0,1].")
      (k_parent_tournament.c_str(), po::value<unsigned>(),
       "Number of individuals chosen at random from the population to "\
       "identify a parent.")
      (k_brood.c_str(), po::value<unsigned>(),
       "Sets the brood size for recombination (0 to disable).")
      (k_dss.c_str(), po::value<std::string>(),
       "Turn on/off the Dynamic Subset Selection algorithm.")
      (k_g_since_start_g.c_str(), po::value<unsigned>(),
       "Sets the maximum number of generations in a run.")
      (k_gwi.c_str(), po::value<unsigned>(),
       "Sets the maximum number of generations without improvement in a run "\
       "(0 disable).")
      ("runs,r",
       po::value(&runs),
       "Number of runs to be tried.")
      (k_mate_zone.c_str(), po::value<unsigned>(),
       "Mating zone. 0 for panmictic.")
      (k_arl.c_str(), po::value<std::string>(),
       "Adaptive Representation through Learning.");

    po::options_description statistics("Statistics");
    statistics.add_options()
      ("stat-dir",
       po::value(&problem.env.stat_dir),
       "Log statistics in 'stat_dir' folder/directory.")
      ("stat-dynamic",
       po::bool_switch(&problem.env.stat_dynamic),
       "Generate a dynamic execution status file.")
      ("stat-summary",
       po::bool_switch(&problem.env.stat_summary),
       "Save a summary of the run")
      ("stat-arl",
       po::bool_switch(&problem.env.stat_arl),
      "Save the list of active ADF");

    po::options_description cmdl_opt(
      "(==(     )==)\n"
      " `-.`. ,',-' \n"
      "    _,-'       " + vita_sr_version1         + "\n" \
      " ,-',' `.`-.   " + vita_sr_version2         + "\n" \
      "(==(     )==)  "                            + "\n" \
      " `-.`. ,',-'   "                            + "\n" \
      "    _,-'\"      "+ "sr [options] data_file" + "\n" \
      " ,-',' `.`-.   "                            + "\n" \
      "(==(     )==)  " + "Allowed options");
    cmdl_opt.add(generic).add(data).add(config).add(evolution).add(individual).
      add(statistics);

    po::positional_options_description p;
    p.add("data", -1);

    po::variables_map vm;

    store(po::command_line_parser(argc, argv).
          options(cmdl_opt).positional(p).run(), vm);

    notify(vm);

    if (vm.count("version"))
    {
      std::cout << vita_sr_version1 << std::endl << vita_sr_version2
                << std::endl;
      return false;
    }

    if (data_file.empty() || vm.count("help"))
    {
      std::cout << cmdl_opt << std::endl;

      if (data_file.empty())
        std::cerr << "Missing data file." << std::endl;

      return false;
    }

    if (vm.count(k_arl))
      problem.env.arl = is_true(vm[k_arl].as<std::string>());
    if (vm.count(k_brood))
      problem.env.brood_recombination = vm[k_brood].as<unsigned>();
    if (vm.count(k_code_length))
      problem.env.code_length = vm[k_code_length].as<unsigned>();
    if (vm.count(k_crossover_rate))
      problem.env.p_cross = vm[k_crossover_rate].as<double>();
    if (vm.count(k_dss))
      problem.env.dss = is_true(vm[k_dss].as<std::string>());
    if (vm.count(k_elitism))
      problem.env.elitism = is_true(vm[k_elitism].as<std::string>());
    if (vm.count(k_g_since_start))
      problem.env.g_since_start = vm[k_g_since_start].as<unsigned>();
    if (vm.count(k_gwi))
      problem.env.g_without_improvement = vm[k_gwi].as<unsigned>();
    if (vm.count(k_mate_zone))
      problem.env.mate_zone = vm[k_mate_zone].as<unsigned>();
    if (vm.count(k_mutation_rate))
      problem.env.p_mutation = vm[k_mutation_rate].as<double>();
    if (vm.count(k_parent_tournament))
      problem.env.par_tournament = vm[k_parent_tournament].as<unsigned>();
    if (vm.count(k_population_size))
      problem.env.individuals = vm[k_population_size].as<unsigned>();

    if (vm.count("random-seed"))
    {
      vita::random::seed(random_seed);
      if (verbose)
        std::cout << "Random seed is " << random_seed << std::endl;
    }

    if (verbose && !problem.env.stat_dir.empty())
      std::cout << "Statistics/status files directory is "
                << problem.env.stat_dir << std::endl;
  }
  catch(std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return false;
  }

  // Reading dataset...
  if (verbose)
    std::cout << "Reading data file (" << data_file << ")... ";
  unsigned parsed(0);
  try
  {
    parsed = problem.load_data(data_file);
  }
  catch(...)
  {
    parsed = 0;
  }
  if (!parsed)
  {
    if (verbose)
      std::cout << "Dataset file format error." << std::endl;
    return false;
  }
  if (verbose)
    std::cout << "ok (" << parsed << " instance(s)" << ")" << std::endl
              << "  [" << problem.categories() << " category(ies), "
              << problem.variables() << " variable(s), "
              << problem.classes() << " class(es)]" << std::endl;

  // ... and the symbol file (if available).
  if (symbol_file.empty())
  {
    if (verbose)
      std::cout << "  [default symbol set]" << std::endl;
    problem.setup_default_symbols();
  }
  else
  {
    if (verbose) std::cout << "Reading symbol file (" << symbol_file
                           << ")... ";
    try
    {
      parsed = problem.load_symbols(symbol_file);
    }
    catch(...)
    {
      parsed = 0;
    }
    if (!parsed)
    {
      if (verbose)
        std::cerr << "Symbol file format error." << std::endl;
      return false;
    }
    if (verbose)
      std::cout << "ok (" << parsed << " symbols)" << std::endl;
  }

  if (!problem.env.sset.enough_terminals())
  {
    std::cerr << std::endl << "Too few terminals." << std::endl;
    return false;
  }

  return true;
}

///
/// fix_parameters
///
void fix_parameters()
{
  if (problem.env.code_length &&
      *problem.env.code_length <= problem.categories())
  {
    const unsigned new_length(2 * problem.categories());
    std::cout << "[WARNING] Adjusting code length (" << *problem.env.code_length
              << " => " << new_length << ')' << std::endl;
    problem.env.code_length = new_length;
  }

  if (problem.env.dss && problem.get_data()->size() <= 10)
  {
    std::cout << "[WARNING] Adjusting DSS (true => false)" << std::endl;
    problem.env.dss = false;
  }

  if (problem.env.p_mutation)
  {
    if (*problem.env.p_mutation < 0.0)
    {
      std::cout << "[WARNING] Adjusting mutation probability ("
                << *problem.env.p_mutation << " => 0.0)" << std::endl;
      problem.env.p_mutation = 0.0;
    }
    else if (*problem.env.p_mutation > 1.0)
    {
      std::cout << "[WARNING] Adjusting mutation probability ("
                << *problem.env.p_mutation << " => 1.0)" << std::endl;
      problem.env.p_mutation = 1.0;
    }
  }

  if (problem.env.p_cross)
  {
    if (*problem.env.p_cross < 0.0)
    {
      std::cout << "[WARNING] Adjusting crossover probability ("
                << *problem.env.p_cross << " => 0.0)" << std::endl;
      problem.env.p_cross = 0.0;
    }
    else if (*problem.env.p_cross > 1.0)
    {
      std::cout << "[WARNING] Adjusting crossover probability ("
                << *problem.env.p_cross << " => 1.0)" << std::endl;
      problem.env.p_cross = 0.0;
    }
  }
}

///
/// \return
///
bool run()
{
  fix_parameters();

  vita::search s(&problem);
  s.run(verbose, runs);

  return true;
}

int main(int argc, char *argv[])
{
  if (!parse_command_line(argc, argv))
    return EXIT_FAILURE;

  run();

  return EXIT_SUCCESS;
}
