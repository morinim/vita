/**
 *
 *  \file sr.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
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

#include "command_line_interpreter.h"

const std::string vita_sr_version1(
  "Vita - Symbolic Regression and classification v0.9.2"
);
const std::string vita_sr_version2(
  "Copyright 2011-2012 EOS di Manlio Morini (http://www.eosdev.it)"
);

vita::src_problem problem;

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

  if (problem.env.dss && problem.data()->size() <= 10)
  {
    std::cout << "[WARNING] Adjusting DSS (true => false)" << std::endl;
    problem.env.dss = false;
  }

  if (problem.env.tournament_size)
  {
    if (*problem.env.tournament_size < 2)
    {
      std::cout << "[WARNING] Adjusting tournament size (=> 2)"
                << std::endl;
      problem.env.tournament_size = 2;
    }

    if (problem.env.mate_zone &&
        *problem.env.tournament_size > *problem.env.mate_zone)
    {
      std::cout << "[WARNING] Adjusting tournament size ("
                << *problem.env.tournament_size << " => "
                << *problem.env.mate_zone << ")" << std::endl;
      problem.env.tournament_size = *problem.env.mate_zone;
    }

    if (problem.env.individuals &&
        *problem.env.tournament_size > *problem.env.individuals)
    {
      std::cout << "[WARNING] Adjusting tournament size ("
                << *problem.env.tournament_size << " => "
                << *problem.env.individuals << ")" << std::endl;
      problem.env.tournament_size = *problem.env.individuals;
    }
  }

  if (!problem.threashold.fitness && !problem.threashold.accuracy)
  {
    if (problem.classes() > 1)  // classification
    {
      problem.threashold.fitness  = boost::none;
      problem.threashold.accuracy =        0.99;

      std::cout << "[WARNING] Adjusting accuracy threashold (=>"
                << problem.threashold.accuracy << ")" << std::endl;
    }
    else  // symbolic regression
    {
      problem.threashold.fitness  =     -0.0001;
      problem.threashold.accuracy = boost::none;

      std::cout << "[WARNING] Adjusting fitness threashold (=>"
                << problem.threashold.fitness << ")" << std::endl;
    }
  }
}

///
/// \param[in] s a value to be interpreted as boolean.
/// \return \c true if \c s is not zero and not "false".
///
bool is_true(const std::string &s)
{
  return s != "0" && !boost::iequals(s, "false");
}

///
/// Contains the text-based command line interface. The interface is not
/// intented to be human friendly (no code completion, command history,
/// submenu...). We just want a light interface for the Python GUI and a simple
/// way to debug.
///
namespace ui
{
  const std::string header(
    "(==(     )==)\n"
    " `-.`. ,',-' \n"
    "    _,-'       " + vita_sr_version1          + "\n" \
    " ,-',' `.`-.   " + vita_sr_version2          + "\n" \
    "(==(     )==)\n"
    " `-.`. ,',-' \n"
    "    _,-'\"   \n"
    " ,-',' `.`-. \n"
    "(==(     )==)\n");

  po::options_description cmdl_opt(
    "sr [options] [dataset]\n\nAllowed options");

  /// Number of runs to be tried.
  unsigned runs(1);

  /// The verbosity level.
  bool verbose(true);

  ///
  /// \param[in] v a value for ARL (\see is_true).
  ///
  /// Sets Adaptive Representation through Learning.
  ///
  void arl(const std::string &v)
  {
    problem.env.arl = is_true(v);

#if !defined(NDEBUG)
    if (verbose)
      std::cout << "Adaptive Representation through Learning is " << v
                << std::endl;
#endif
  }

  ///
  /// \param[in] size brood size for recombination (0 to disable).
  ///
  /// Sets the brood size for recombination.
  ///
  void brood(unsigned size)
  {
    problem.env.brood_recombination = size;

    if (verbose)
      std::cout << "Brood size for recombination is " << size << std::endl;
  }

  ///
  /// \param[in] length code/genome length of an individual.
  ///
  /// Sets the code/genome length of an individual.
  ///
  void code_length(unsigned length)
  {
    assert(length);
    problem.env.code_length = length;

    if (verbose)
      std::cout << "Code length is " << length << std::endl;
  }

  ///
  /// \param[in] r range is [0,1].
  ///
  /// Sets the overall probability that crossover will occour between winners
  /// in a tournament.
  ///
  void crossover_rate(double r)
  {
    if (r < 0.0)
    {
      r = 0.0;
      std::cout << "[WARNING] Adjusting crossover probability (" << r
                << " => 0.0)" << std::endl;
    }
    else if (*problem.env.p_cross > 1.0)
    {
      r = 1.0;
      std::cout << "[WARNING] Adjusting crossover probability (" << r
                << " => 1.0)" << std::endl;
    }

    problem.env.p_cross = r;

    if (verbose)
      std::cout << "Crossover rate is " << r << std::endl;
  }

  ///
  /// \param[in] data_file guess what? This is the data file to be parsed.
  /// \return \c true if data have been correctly read and parsed.
  ///
  bool data(const std::string &data_file)
  {
    if (verbose)
      std::cout << "Reading data file (" << data_file << ")... ";

    unsigned parsed(0);
    try
    {
      parsed = problem.load(data_file).first;
    }
    catch(...)
    {
      parsed = 0;
    }

    if (verbose)
    {
      if (!parsed)
        std::cerr << "dataset file format error." << std::endl;
      else
        std::cout << "ok" << std::endl << "  [examples: " << parsed
                  << ", categories: " << problem.categories()
                  << ", features: " << problem.variables()
                  << ", classes: " << problem.classes() << "]" << std::endl;
    }

    return parsed;
  }

  ///
  /// \param[in] v a value for DSS (\see is_true).
  ///
  /// Turn on/off the Dynamic Subset Selection algorithm.
  ///
  void dss(const std::string &v)
  {
    problem.env.dss = is_true(v);

    if (verbose)
      std::cout << "Dynamic Subset Selection is " << v << std::endl;
  }

  ///
  /// \param[in] v a value for elitism (\see is_true).
  ///
  void elitism(const std::string &v)
  {
    problem.env.elitism = is_true(v);

    if (verbose)
      std::cout << "Elitism is " << v << std::endl;
  }

  ///
  /// Shows the environment.
  ///
  void environment(bool)
  {
    std::cout << "NOT READY" << std::endl;
  }

  ///
  /// Quit program.
  ///
  void exit(bool)
  {
    std::exit(EXIT_SUCCESS);

    if (verbose)
      std::cout << "Bye." << std::endl;
  }

  ///
  /// \param[in] g maximum number of generations in a run.
  ///
  /// Sets the maximum number of generations in a run.
  ///
  void generations(unsigned g)
  {
    problem.env.g_since_start = g;

    if (verbose)
      std::cout << "Generations is " << g << std::endl;
  }

  ///
  /// Starts the search.
  ///
  void go(bool = true)
  {
    if (problem.data()->size())
      if (problem.env.sset.enough_terminals())
      {
        fix_parameters();

        vita::search s(&problem);
        s.run(verbose, runs);
      }
      else
        std::cerr << "Too few terminals." << std::endl;
    else
      std::cerr << "Missing dataset." << std::endl;
  }

  ///
  /// \param[in] g maximum number of generations without improvement in a run
  ///              (0 to disable).
  ///
  /// Sets the maximum number of generations without improvement in a run.
  void gwi(unsigned g)
  {
    problem.env.g_without_improvement = g;

    if (verbose)
      std::cout << "Maximum number of generations without improvement is "
                << g << std::endl;
  }

  ///
  /// Shows the help screen.
  ///
  void help(bool)
  {
    std::cout << cmdl_opt << std::endl;
  }

  ///
  /// \param[in] z mating zone (0 for panmictic).
  ///
  /// Set mating zone.
  ///
  void mate_zone(unsigned z)
  {
    problem.env.mate_zone = z;

    if (verbose)
      std::cout << "Mate zone is " << z << std::endl;
  }

  ///
  /// \param[in] r mutation rate, range is [0,1].
  ///
  /// Sets the overall probability of mutation of the individuals that have
  /// been selected as winners in a tournament.
  ///
  void mutation_rate(double r)
  {
    if (r < 0.0)
    {
      r = 0.0;
      std::cout << "[WARNING] Adjusting mutation probability (" << r
                << " => 0.0)" << std::endl;
    }
    else if (r > 1.0)
    {
      r = 1.0;
      std::cout << "[WARNING] Adjusting mutation probability (" << r
                << " => 1.0)" << std::endl;
    }

    problem.env.p_mutation = r;

    if (verbose)
      std::cout << "Mutation rate is " << r << std::endl;
  }

  ///
  /// \param[in] n number of individuals.
  ///
  /// Sets the number of individuals examined for choosing parents.
  ///
  void tournament_size(unsigned n)
  {
    problem.env.tournament_size = n;

    if (verbose)
      std::cout << "Tournament size is " << n << std::endl;
  }

  ///
  /// \param[in] size number of programs/individuals in the population.
  ///
  /// Sets the number of programs/individuals in the population.
  ///
  void population_size(unsigned size)
  {
    assert(size);
    problem.env.individuals = size;

    if (verbose)
      std::cout << "Population size is " << size << std::endl;
  }

  ///
  /// \param[in] seed sets the seed for the pseudo-random number generator.
  ///                 Pseudo-random sequences are repeatable by using the same
  ///                 seed value.
  ///
  void random_seed(unsigned seed)
  {
    vita::random::seed(seed);

    if (verbose)
      std::cout << "Random seed is " << seed << std::endl;
  }

  ///
  /// \param[in] r number of runs.
  ///
  /// Number of runs to be tried.
  ///
  void set_runs(unsigned r)
  {
    assert(r);
    runs = r;

    if (verbose)
      std::cout << "Number of runs is " << r << std::endl;
  }

  ///
  /// \param[in] v if \c true saves the list of active ADFs.
  ///
  void stat_arl(const std::string &v)
  {
    problem.env.stat_arl = true;

    if (verbose)
      std::cout << "ARL is " << v << std::endl;
  }

  ///
  /// \param[in] dir a directory.
  ///
  /// Logs statistics in \c stat_dir folder/directory.
  ///
  void stat_dir(const std::string &dir)
  {
    problem.env.stat_dir = dir;

    if (verbose)
      std::cout << "Logging folder is " << dir << std::endl;
  }

  ///
  /// \param[in] v should we save the dynamic execution status file?
  ///
  void stat_dynamic(const std::string &v)
  {
    problem.env.stat_dynamic = is_true(v);

    if (verbose)
      std::cout << "Dynamic evolution logging is " << v << std::endl;
  }

  ///
  /// \param[in] v if \c true saves a summary of the runs.
  ///
  void stat_summary(const std::string &v)
  {
    problem.env.stat_summary = true;

    if (verbose)
      std::cout << "Summary logging is " << v << std::endl;
  }

  ///
  /// \param[in] symbol_file file containing a list of symbols (functions and
  ///                        terminals). If \c symbol_file is \c empty(), the
  ///                        default symbol set will be loaded.
  /// \return \c true if symbols have been correctly read and parsed.
  ///
  bool symbols(const std::string &symbol_file)
  {
    if (symbol_file.empty())
    {
      if (verbose)
        std::cout << "Using default symbol set." << std::endl;
      problem.setup_default_symbols();
    }
    else
    {
      if (verbose) std::cout << "Reading symbol file (" << symbol_file
                             << ")... ";

      unsigned parsed(0);
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
          std::cerr << "symbol file format error." << std::endl;
        return false;
      }
      if (verbose)
        std::cout << "ok" << std::endl << "  [symbols: " << parsed << "]"
                  << std::endl;
    }

    if (!problem.env.sset.enough_terminals())
    {
      std::cerr << "Too few terminals." << std::endl;
      return false;
    }

    return true;
  }

  ///
  /// \param[in] v the threashold value
  ///
  /// If the output value of a run is greater than \a v it's scored as a
  /// success. The output value considered is the fitness when \a v is a simple
  /// number or the accuracy when \a v is a percentage
  ///
  void threashold(const std::string &v)
  {
    problem.threashold.accuracy = boost::none;
    problem.threashold.fitness  = boost::none;

    if (*v.rbegin() == '%')
    {
      const double accuracy(std::stod(v) / 100.0);
      assert(0.0 <= accuracy && accuracy <= 1.0);

      problem.threashold.accuracy = accuracy;
    }
    else
    {
      const vita::fitness_t fitness(std::stod(v));
      assert(fitness <= 0.0);

      problem.threashold.fitness = fitness;
    }

    if (verbose)
      std::cout << "Threashold is " << v << std::endl;
  }

  ///
  /// \param[in] bits number of bits used for the ttable (ttable contains
  ///                 \c 2^bits elements).
  ///
  void ttable(unsigned bits)
  {
    assert(bits);
    problem.env.ttable_size = bits;

    if (verbose)
      std::cout << "TTable size is " << bits << " bits" << std::endl;
  }

  ///
  /// \param[in] v verbosity level.
  ///
  void verbosity(bool v)
  {
    verbose = v;

    if (verbose)
      std::cout << "Verbosity is " << v << std::endl;
  }
}  // namespace ui

///
/// \param[in] argc
/// \param[in] argv
/// \return -1 if command line parsing is enough (no further computing),
///          0 in case of errors,
///          1 if further computing is required.
///
int parse_command_line(int argc, char *const argv[])
{
  try
  {
    // Declare a group of options that will be allowed only on command line.
    po::options_description generic("Generic");
    generic.add_options()
      ("version,v", "print version string")
      ("help,h", po::value<bool>()->zero_tokens()->notifier(&ui::help),
       "produce the help message")
      ("quiet",
       po::value<unsigned>()->zero_tokens()->implicit_value(0)->notifier(&ui::verbosity),
       "turn off verbosity")
      ("verbose",
       po::value<unsigned>()->zero_tokens()->implicit_value(1)->notifier(&ui::verbosity),
       "turn on verbosity");

    po::options_description data("Data");
    data.add_options()
      ("data,d", po::value<std::string>()->notifier(&ui::data), "dataset")
      ("symbols,s",
       po::value<std::string>()->implicit_value("")->notifier(&ui::symbols),
       "symbols file");

    po::options_description config("Config");
    config.add_options()
      ("ttable",
       po::value<unsigned>()->notifier(&ui::ttable),
       "number of bits used for the ttable (ttable contains 2^bits elements)")
      ("random-seed",
       po::value<unsigned>()->notifier(&ui::random_seed),
       "sets the seed for the pseudo-random number generator. "\
       "Pseudo-random sequences are repeatable by using the same seed value");

    // Declare a group of options that will be allowed both on command line
    // and in config file.
    po::options_description individual("Individual");
    individual.add_options()
      ("code-length,l", po::value<unsigned>()->notifier(&ui::code_length),
       "sets the code/genome length of an individual");

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
      ("population-size,P",
       po::value<unsigned>()->notifier(&ui::population_size),
       "sets the number of programs/individuals in the population")
      ("elitism", po::value<std::string>()->notifier(&ui::elitism),
       "when elitism is true an individual will never replace a better one")
      ("mutation-rate", po::value<double>()->notifier(&ui::mutation_rate),
       "sets the overall probability of mutation of the individuals that have "\
       "been selected as winners in a tournament. Range is [0,1]")
      ("crossover-rate,c", po::value<double>()->notifier(&ui::crossover_rate),
       "sets the overall probability that crossover will occour between "\
       "winners in a tournament. Range is [0,1]")
      ("tournament-size",
       po::value<unsigned>()->notifier(&ui::tournament_size),
       "number of individuals examined for choosing parents")
      ("brood", po::value<unsigned>()->notifier(&ui::brood),
       "sets the brood size for recombination (0 to disable)")
      ("dss", po::value<std::string>()->notifier(&ui::dss),
       "turns on/off the Dynamic Subset Selection algorithm")
      ("generations,g", po::value<unsigned>()->notifier(&ui::generations),
       "sets the maximum number of generations in a run")
      ("gwi", po::value<unsigned>()->notifier(&ui::gwi),
       "sets the maximum number of generations without improvement in a run "\
       "(0 disable)")
      ("runs,r", po::value<unsigned>()->notifier(&ui::set_runs),
       "number of runs to be tried")
      ("mate-zone", po::value<unsigned>()->notifier(&ui::mate_zone),
       "mating zone (0 for panmictic)")
      ("arl", po::value<std::string>()->notifier(&ui::arl),
       "adaptive Representation through Learning");

    po::options_description statistics("Statistics");
    statistics.add_options()
      ("stat-dir", po::value<std::string>()->notifier(&ui::stat_dir),
       "log statistics in 'stat_dir' folder/directory")
      ("stat-dynamic", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_dynamic),
       "generates a dynamic execution status file")
      ("stat-summary", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_summary),
       "saves a summary of the runs")
      ("stat-arl", po::value<std::string>()->notifier(&ui::stat_arl),
       "saves the list of active ADFs")
      ("threashold", po::value<std::string>()->notifier(&ui::threashold),
       "sets the success threashold for a run");

    ui::cmdl_opt.add(generic).add(data).add(config).add(evolution).
      add(individual).add(statistics);

    po::positional_options_description p;
    p.add("data", 1);

    po::variables_map vm;

    store(po::command_line_parser(argc, argv).
          options(ui::cmdl_opt).positional(p).run(), vm);

    notify(vm);

    if (vm.count("version"))
    {
      std::cout << vita_sr_version1 << std::endl << vita_sr_version2
                << std::endl;
      return -1;
    }

    if (vm.count("help"))
      return -1;
  }
  catch(std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 0;
  }

  return 1;
}

int main(int argc, char *const argv[])
{
  std::cout << ui::header << std::endl;

  const int ret(parse_command_line(argc, argv));
  if (ret < 0)  // help or version options
    return EXIT_SUCCESS;

  if (ret == 0)  // error
    return EXIT_FAILURE;

  assert(ret > 0);

  if (problem.data()->size())
    ui::go();
  else
  {
    po::options_description interface("Interface");
    interface.add_options()
      ("go", po::value<bool>()->zero_tokens()->notifier(&ui::go),
       "let's go!")
      ("environment",
       po::value<bool>()->zero_tokens()->notifier(&ui::environment),
       "show the environment")
      ("exit", po::value<bool>()->zero_tokens()->notifier(&ui::exit),
       "quit the program");
    ui::cmdl_opt.add(interface);

    boost::cli::command_line_interpreter cli(ui::cmdl_opt, "> ");
    cli.interpret(std::cin);
  }

  return EXIT_SUCCESS;
}
