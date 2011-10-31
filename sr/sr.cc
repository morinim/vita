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

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "kernel/environment.h"
#include "kernel/search.h"
#include "kernel/src_problem.h"

const std::string vita_sr_version1(
  "Vita - Symbolic Regression and classification v0.9.1"
);
const std::string vita_sr_version2(
  "Copyright (c) 2011 EOS Development (http://www.eosdev.it)"
);

bool verbose(false);

unsigned runs(1);


vita::src_problem problem(-5.0);

void setup_default_symbols()
{
  vita::symbol_ptr c1(new vita::sr::constant(1));
  vita::symbol_ptr c2(new vita::sr::constant(2));
  vita::symbol_ptr c3(new vita::sr::constant(3));
  vita::symbol_ptr c4(new vita::sr::constant(4));
  vita::symbol_ptr c5(new vita::sr::constant(5));
  vita::symbol_ptr c6(new vita::sr::constant(6));
  vita::symbol_ptr c7(new vita::sr::constant(7));
  vita::symbol_ptr c8(new vita::sr::constant(8));
  vita::symbol_ptr c9(new vita::sr::constant(9));

  problem.env.insert(c1);
  problem.env.insert(c2);
  problem.env.insert(c3);
  problem.env.insert(c4);
  problem.env.insert(c5);
  problem.env.insert(c6);
  problem.env.insert(c7);
  problem.env.insert(c8);
  problem.env.insert(c9);

  vita::symbol_ptr s_abs(new vita::sr::abs());
  vita::symbol_ptr s_add(new vita::sr::add());
  vita::symbol_ptr s_div(new vita::sr::div());
  vita::symbol_ptr s_ln(new vita::sr::ln());
  vita::symbol_ptr s_mul(new vita::sr::mul());
  vita::symbol_ptr s_mod(new vita::sr::mod());
  vita::symbol_ptr s_sub(new vita::sr::sub());

  problem.env.insert(s_abs);
  problem.env.insert(s_add);
  problem.env.insert(s_div);
  problem.env.insert(s_ln);
  problem.env.insert(s_mul);
  problem.env.insert(s_mod);
  problem.env.insert(s_sub);
}

///
/// \param[in] argc
/// \param[in] argv
/// \return
///
bool parse_command_line(int argc, char *argv[])
{
  unsigned random_seed;
  std::string data_file, f_f, symbols_file;

  try
  {
    // Declare a group of options that will be allowed only on command line.
    po::options_description generic("Generic options");
    generic.add_options()
      ("version,v", "print version string.")
      ("help,h", "produces help message.")
      ("verbose", po::bool_switch(&verbose), "shows more details.");

    po::options_description data("Data");
    data.add_options()
      ("data,d", po::value(&data_file), "Data file.")
      ("symbols,s", po::value(&symbols_file), "Symbols file.");

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
      ("force-input,f",
       po::value<bool>(&problem.env.force_input)->default_value(
         problem.env.force_input),
       "Include all the input variables in every generated individual.")
      ("program-size,p",
       po::value(&problem.env.code_length)->default_value(
         problem.env.code_length),
       "Sets the maximum length of a program (it might be shorter)");

    // Declare a group of options that will be allowed both on command line
    // and in config file.
    po::options_description evolution("Evolution");
    evolution.add_options()
      ("population-size,P",
       po::value(&problem.env.individuals)->default_value(
         problem.env.individuals),
       "Sets the number of programs/individuals in the population.")
      ("elitism",
       po::value<bool>(&problem.env.elitism)->default_value(true),
       "When elitism is true an individual will never replace a better one.")
      ("mutation-rate,m",
       po::value(&problem.env.p_mutation)->default_value(
         problem.env.p_mutation),
       "Sets the overall probability of mutation of the individuals that have "\
       "been selected as winners in a tournament. Range is [0,1].")
      ("crossover-rate,c",
       po::value(&problem.env.p_cross)->default_value(
         problem.env.p_cross),
       "Sets the overall probability that crossover will occour between two "\
       "winners in a tournament. Range is [0,1].")
      ("parent-tournament,T",
       po::value(&problem.env.par_tournament)->default_value(
         problem.env.par_tournament),
       "Number of individuals chosen at random from the population to "\
       "identify a parent.")
      ("brood",
       po::value(&problem.env.brood_recombination)->default_value(
         problem.env.brood_recombination),
       "Sets the brood size for recombination (-1 auto-select, 0 disable).")
      ("g-since-start,g",
       po::value(&problem.env.g_since_start)->default_value(
         problem.env.g_since_start),
       "Sets the maximum number of generations in a run.")
      ("gwi",
       po::value(&problem.env.g_without_improvement)->default_value(
         problem.env.g_without_improvement),
       "Sets the maximum number of generations without improvement in a run "\
       "(0 disable).")
      ("runs,r",
       po::value(&runs),
       "Number of runs to be tried.")
      ("mate-zone",
       po::value(&problem.env.mate_zone)->default_value(
         problem.env.mate_zone),
       "Mating zone. 0 for panmictic.")
      ("arl",
       po::bool_switch(&problem.env.arl),
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
    std::cout << parsed << " lines" << std::endl
              << "  (" << problem.variables() << " variables, "
              << problem.classes() << " classes)" << std::endl;

  if (symbols_file.empty())
  {
    if (verbose)
      std::cout << "  (default symbol set)" << std::endl;
    setup_default_symbols();
  }
  else
  {
    if (verbose) std::cout << "Reading symbols' file (" << symbols_file
                           << ")... ";
    const std::string s(problem.load_symbols(symbols_file));
    if (s.empty())
      return false;
    if (verbose)
      std::cout << "ok" << std::endl
                << "  (" << s << ')' << std::endl;
  }

  return true;
}

///
/// \return
///
bool run()
{
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
