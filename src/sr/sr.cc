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

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "kernel/environment.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/problem.h"
#include "kernel/src/primitive/factory.h"
#include "kernel/src/search.h"

#include "command_line_interpreter.h"

namespace po = boost::program_options;

namespace
{
const std::string vita_sr_version1(
  "Vita - Symbolic Regression and classification v0.9.10"
  );
const std::string vita_sr_version2(
  "Copyright 2011-2014 EOS di Manlio Morini (http://www.eosdev.it)"
  );
const std::string vita_sr_defs(
#if defined(CLONE_SCALING)
  "(clone scaling enabled)"
#else
  ""
#endif
  );
}

///
/// Fixes conflicting parameters.
///
void fix_parameters(vita::src_problem *const problem)
{
  vita::environment &env(problem->env);

  if (env.code_length && env.code_length <= problem->categories())
  {
    const decltype(env.code_length) new_length(2 * problem->categories());
    std::cout << vita::k_s_warning << " Adjusting code length ("
              << env.code_length << " => " << new_length << ")\n";
    env.code_length = new_length;
  }

  if (env.dss && problem->data()->size() <= 30)
  {
    std::cout << vita::k_s_warning << " Adjusting DSS (true => false)\n";
    env.dss = false;
  }

  if (env.tournament_size)
  {
    if (env.tournament_size < 2)
    {
      std::cout << vita::k_s_warning << " Adjusting tournament size (=> 2)\n";
      env.tournament_size = 2;
    }

    if (env.mate_zone && env.tournament_size > *env.mate_zone)
    {
      std::cout << vita::k_s_warning << " Adjusting tournament size ("
                << env.tournament_size << " => " << *env.mate_zone << ")\n";
      env.tournament_size = *env.mate_zone;
    }

    if (env.individuals && env.tournament_size > env.individuals)
    {
      std::cout << vita::k_s_warning << " Adjusting tournament size ("
                << env.tournament_size << " => " << env.individuals << ")\n";
      env.tournament_size = env.individuals;
    }
  }

  if (env.threshold.fitness == vita::fitness_t() &&
      env.threshold.accuracy < 0.0)
  {
    if (problem->classification())
    {
      env.threshold.accuracy = 0.99;

      std::cout << vita::k_s_info << " Accuracy threshold set to "
                << env.threshold.accuracy << '\n';
    }
    else  // symbolic regression
    {
      env.threshold.fitness = {-0.0001};

      std::cout << vita::k_s_info << " Fitness threshold set to "
                << env.threshold.fitness << '\n';
    }
  }
}

///
/// \param[in] s a value to be interpreted as boolean.
/// \return `true` if `s` is not zero and not `"false"`.
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
  "    _,-'       " + vita_sr_version1 + "\n"   \
  " ,-',' `.`-.   " + vita_sr_version2 + "\n"   \
  "(==(     )==)\n"
  " `-.`. ,',-'   " + vita_sr_defs + "\n"       \
  "    _,-'\"   \n"
  " ,-',' `.`-. \n"
  "(==(     )==)\n");

po::options_description cmdl_opt(
  "sr [options] [data set]\n\nAllowed options");

/// Number of runs to be tried.
unsigned runs(1);

// Active evaluator.
vita::evaluator_id eva(vita::evaluator_id::undefined);
std::string eva_args;

/// Reference problem (the problem we will work on).
vita::src_problem *problem;

///
/// \param[in] v a value for ARL (\see is_true).
///
/// Sets Adaptive Representation through Learning.
///
void arl(const std::string &v)
{
  problem->env.arl = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info
              << " Adaptive Representation through Learning is "
              << problem->env.arl << '\n';
}

///
/// \param[in] size brood size for recombination (0 to disable).
///
/// Sets the brood size for recombination.
///
void brood(unsigned size)
{
  problem->env.brood_recombination = size;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Brood size for recombination set to "
              << size << '\n';
}

///
/// \param[in] length code/genome length of an individual.
///
/// Sets the code/genome length of an individual.
///
void code_length(unsigned length)
{
  assert(length);
  problem->env.code_length = length;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Code length set to " << length << '\n';
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
    std::cout << vita::k_s_warning << " Adjusting crossover probability ("
              << r << " => 0.0)\n";
  }
  else if (r > 1.0)
  {
    r = 1.0;
    std::cout << vita::k_s_warning << " Adjusting crossover probability ("
              << r << " => 1.0)\n";
  }

  problem->env.p_cross = r;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Crossover rate set to " << r << '\n';
}

///
/// \param[in] data_file guess what? This is the data file to be parsed.
/// \return `true` if data have been correctly read and parsed.
///
bool data(const std::string &data_file)
{
  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Reading data file " << data_file
              << "...\n";

  std::size_t parsed(0);
  try
  {
    parsed = problem->load(data_file).first;
  }
  catch(...)
  {
    parsed = 0;
  }

  if (!parsed)
    std::cerr << vita::k_s_error << " Dataset file format error\n";
  else if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Dataset read. Examples: " << parsed
              << ", categories: " << problem->categories()
              << ", features: " << problem->variables()
              << ", classes: " << problem->classes() << '\n';

  return parsed;
}

///
/// \param[in] v a value for DSS (\see is_true).
///
/// Turn on/off the Dynamic Subset Selection algorithm.
  ///
void dss(const std::string &v)
{
  problem->env.dss = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Dynamic Subset Selection is "
              << problem->env.dss << '\n';
}

///
/// \param[in] v a value for elitism (\see is_true).
///
void elitism(const std::string &v)
{
  problem->env.elitism = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Elitism is " << problem->env.elitism
              << '\n';
}

///
/// Shows the environment.
///
void environment(bool)
{
  std::cout << "NOT READY\n";
}

///
/// param[in] v the chosen evaluator.
///
/// \note
/// The evaluator should fit the current problem kind (symbolic regression,
/// classification) or it won't be used.
///
void evaluator(const std::string &v)
{
  const auto sep(v.find(':'));
  const std::string keyword(v.substr(0, sep));

  eva_args = "";
  if (sep != std::string::npos && sep + 1 < v.size())
    eva_args = v.substr(sep + 1);

  bool ok(true);
  if (keyword == "count")
    eva = vita::evaluator_id::count;
  else if (keyword == "mae")
    eva = vita::evaluator_id::mae;
  else if (keyword == "rmae")
    eva = vita::evaluator_id::rmae;
  else if (keyword == "mse")
    eva = vita::evaluator_id::mse;
  else if (keyword == "binary")
    eva = vita::evaluator_id::bin;
  else if (keyword == "dynslot")
    eva = vita::evaluator_id::dyn_slot;
  else if (keyword == "gaussian")
    eva = vita::evaluator_id::gaussian;
  else
    ok = false;

  if (ok)
  {
    std::cout << vita::k_s_info << " Evaluator is " << keyword;
    if (!eva_args.empty())
      std::cout << " (parameters: " << eva_args << ")";
    std::cout << '\n';
  }
  else
    std::cerr << vita::k_s_error << " Wrong argument for evaluator command\n";
}

///
/// Quit program.
///
void exit(bool)
{
  if (problem->env.verbosity >= 2)
    std::cout << "Bye\n";

  std::exit(EXIT_SUCCESS);
}

///
/// \param[in] g maximum number of generations in a run.
///
/// Sets the maximum number of generations in a run.
///
void generations(unsigned g)
{
  problem->env.generations = g;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Generations set to " << g << '\n';
}

///
/// Starts the search.
///
void go(bool = true)
{
  if (problem->data()->size())
    if (problem->sset.enough_terminals())
    {
      fix_parameters(problem);

      vita::src_search<vita::i_mep, vita::alps_es> s(*problem);

      if (eva != vita::evaluator_id::undefined)
        s.set_evaluator(eva, eva_args);

      s.run(runs);
    }
    else
      std::cerr << vita::k_s_error << " Too few terminals\n";
  else
    std::cerr << vita::k_s_error << " Missing data set\n";
}

///
/// \param[in] g maximum number of generations without improvement in a run
///              (0 to disable).
///
/// Sets the maximum number of generations without improvement in a run.
void gwi(unsigned g)
{
  problem->env.g_without_improvement = g;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info
              << " Max number of generations without improvement set to "
              << g << '\n';
}

///
/// Shows the help screen.
///
void help(bool)
{
  std::cout << cmdl_opt << '\n';
}

///
/// \param[in] z mating zone (0 for panmictic).
///
/// Set mating zone.
///
void mate_zone(unsigned z)
{
  problem->env.mate_zone = z;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Mate zone set to " << z << '\n';
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
    std::cout << vita::k_s_warning << " Adjusting mutation probability ("
              << r << " => 0.0)\n";
  }
  else if (r > 1.0)
  {
    r = 1.0;
    std::cout << vita::k_s_warning << " Adjusting mutation probability ("
              << r << " => 1.0)\n";
  }

  problem->env.p_mutation = r;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Mutation rate set to " << r
              << '\n';
}

///
/// \param[in] ts The dataset used as test set.
/// \return `true` if data have been correctly read and parsed.
///
bool testset(const std::string &ts)
{
  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Reading test set file " << ts << "...\n";

  std::size_t parsed(0);
  try
  {
    parsed = problem->load_test_set(ts);
  }
  catch(...)
  {
    parsed = 0;
  }

  if (!parsed)
    std::cerr << vita::k_s_error << " Test set file format error\n";
  else if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Testset read. Examples: " << parsed
              << ", categories: " << problem->categories()
              << ", features: " << problem->variables()
              << ", classes: " << problem->classes() << '\n';

  return parsed;
}

///
/// \param[in] n number of individuals.
///
/// Sets the number of individuals examined for choosing parents.
///
void tournament_size(unsigned n)
{
  problem->env.tournament_size = n;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Tournament size set to " << n
              << '\n';
}

///
/// \param[in] l number of layers of the population
///
/// Sets the number of layers of the population.
///
void layers(unsigned l)
{
  problem->env.layers = l;

  if (problem->env.verbosity >= 2)
  {
    std::cout << vita::k_s_info << " Number of layers set to ";
    if (l)
      std::cout << l;
    else
      std::cout << "automatic";
    std::cout << '\n';
  }
}

///
/// \param[in] size number of individuals in a layer of the population.
///
/// Sets the number of individuals in a layer of the population.
///
void population_size(unsigned size)
{
  problem->env.individuals = size;

  if (problem->env.verbosity >= 2)
  {
    std::cout << vita::k_s_info << " Population size set to ";
    if (size)
      std::cout << size;
    else
      std::cout << "automatic";
    std::cout << '\n';
  }
}

///
/// \param[in] seed sets the seed for the pseudo-random number generator.
///                 Pseudo-random sequences are repeatable by using the same
///                 seed value.
///
void random_seed(unsigned seed)
{
  vita::random::seed(seed);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Random seed is " << seed << '\n';
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

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Number of runs set to " << r << '\n';
}

///
/// \param[in] v if `true` saves the list of active ADFs.
///
void stat_arl(const std::string &v)
{
  problem->env.stat_arl = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " ARL is " << v << '\n';
}

///
/// \param[in] dir a directory.
///
/// Logs statistics in the `stat_dir` directory.
///
void stat_dir(const std::string &dir)
{
  problem->env.stat_dir = dir;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Logging folder is " << dir << '\n';
}

///
/// \param[in] v should we save the dynamic execution status file?
///
void stat_dynamic(const std::string &v)
{
  problem->env.stat_dynamic = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Dynamic evolution logging is "
              << problem->env.stat_dynamic << '\n';
}

///
/// \param[in] v should we save the layers status file?
///
void stat_layers(const std::string &v)
{
  problem->env.stat_layers = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Layers logging is "
              << problem->env.stat_layers << '\n';
}

///
/// \param[in] v should we save the population status file?
///
void stat_population(const std::string &v)
{
  problem->env.stat_population = is_true(v);

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Population logging is "
              << problem->env.stat_population << '\n';
}

///
/// \param[in] v if `true` saves a summary of the runs.
///
void stat_summary(const std::string &v)
{
  problem->env.stat_summary = true;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Summary logging is " << v << '\n';
}

///
/// \param[in] symbol_file file containing a list of symbols (functions and
///                        terminals). If `symbol_file` is `empty()`, the
///                        default symbol set will be loaded.
/// \return `true` if symbols have been correctly read and parsed.
///
bool symbols(const std::string &symbol_file)
{
  if (symbol_file.empty())
  {
    if (problem->env.verbosity >= 2)
      std::cout << vita::k_s_info << "Using default symbol set\n";
    problem->setup_default_symbols();
  }
  else
  {
    if (problem->env.verbosity >= 2)
      std::cout << vita::k_s_info << " Reading symbol file " << symbol_file
                << "...\n";

    unsigned parsed(0);
    try
    {
      parsed = problem->load_symbols(symbol_file);
    }
    catch(...)
    {
      parsed = 0;
    }

    if (!parsed)
    {
      std::cerr << vita::k_s_error << " Symbol file format error\n";
      return false;
    }
    if (problem->env.verbosity >= 2)
      std::cout << vita::k_s_info << " Symbolset read. Symbols: " << parsed
                << '\n';
  }

  if (!problem->sset.enough_terminals())
  {
    std::cerr << vita::k_s_error << " Too few terminals\n";
    return false;
  }

  return true;
}

///
/// \param[in] v the threshold value
///
/// If the output value of a run is greater than `v` it's scored as a success.
/// The output value considered is the fitness when `v` is a simple number or
/// the accuracy when `v` is a percentage.
///
void threshold(const std::string &v)
{
  bool set(false);

  if (v.length())
  {
    if (v.back() == '%')
    {
      const auto accuracy(std::stod(v.substr(0, v.length() - 1)) / 100.0);

      set = (0.0 < accuracy) && (accuracy <= 1.0);
      if (set)
        problem->env.threshold.accuracy = accuracy;
    }
    else
    {
      const vita::fitness_t::value_type fitness(std::stod(v));

      set = (fitness <= 0.0);
      if (set)
        problem->env.threshold.fitness = {fitness};
    }
  }

  if (problem->env.verbosity >= 2)
  {
    if (set)
      std::cout << vita::k_s_info << " Threshold is " << v << '\n';
    else
      std::cerr << vita::k_s_error << " Invalid threshold value\n";
  }
}

///
/// \param[in] bits number of bits used for the ttable (ttable contains
///                 `2^bits elements`).
///
void ttable(unsigned bits)
{
  assert(bits);
  problem->env.ttable_size = bits;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " TTable size is " << bits << " bits\n";
}

///
/// \param[in] v range is [0,1] or [0%,100%].
///
/// Sets percent of the dataset used for validation.
///
void validation(const std::string &v)
{
  bool set(false);

  if (v.length())
  {
    unsigned percentage;
    if (v.back() == '%')
      percentage =
        static_cast<unsigned>(std::stoul(v.substr(0, v.length() - 1)));
    else
      percentage = static_cast<unsigned>(std::stod(v) * 100.0);

    set = (percentage <= 90);
    if (set)
      problem->env.validation_percentage = percentage;
  }

  if (problem->env.verbosity >= 2)
  {
    if (set)
      std::cout << vita::k_s_info << " Validation set percentage is " << v
                << '\n';
    else
      std::cerr << vita::k_s_error << " Invalid validation percentage\n";
  }
}

///
/// \param[in] v verbosity level.
///
void verbosity(unsigned v)
{
  problem->env.verbosity = v;

  if (problem->env.verbosity >= 2)
    std::cout << vita::k_s_info << " Verbosity is " << v << '\n';
}

}  // namespace ui

///
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
       po::value<unsigned>()->zero_tokens()->implicit_value(0)->notifier(
         &ui::verbosity),
       "turn off verbosity")
      ("verbose",
       po::value<unsigned>()->zero_tokens()->implicit_value(2)->notifier(
         &ui::verbosity),
       "turn on verbosity");

    po::options_description data("Data");
    data.add_options()
      ("data,d", po::value<std::string>()->notifier(&ui::data), "data set")
      ("symbols,s",
       po::value<std::string>()->implicit_value("")->notifier(&ui::symbols),
       "symbols file")
      ("testset,t", po::value<std::string>()->notifier(&ui::testset),
       "test set")
      ("validation", po::value<std::string>()->notifier(&ui::validation),
       "sets the percent of the dataset used for validation");

    po::options_description config("Config");
    config.add_options()
      ("evaluator", po::value<std::string>()->notifier(&ui::evaluator),
       "sets preferred evaluator (count, mae, rmae, mse, binary, dynslot, gaussian)")
      ("random-seed", po::value<unsigned>()->notifier(&ui::random_seed),
       "sets the seed for the pseudo-random number generator. "\
       "Pseudo-random sequences are repeatable by using the same seed value")
      ("ttable", po::value<unsigned>()->notifier(&ui::ttable),
       "number of bits used for the ttable (ttable contains 2^bits elements)");

    // Declare a group of options that will be allowed both on command line
    // and in config file.
    po::options_description individual("Population/Individual");
    individual.add_options()
      ("population-size,P",
       po::value<unsigned>()->notifier(&ui::population_size),
       "sets the number of individuals in a layer of the population")
      ("layers,L", po::value<unsigned>()->notifier(&ui::layers),
       "sets the number of layers of the population")
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
       "log statistics in the specified folder/directory")
      ("stat-arl", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_arl),
       "saves the list of active ADFs")
      ("stat-dynamic", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_dynamic),
       "generates a dynamic execution status file")
      ("stat-layers", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_layers),
       "generates a layers status file")
      ("stat-population", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_population),
       "generates a population status file")
      ("stat-summary", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_summary),
       "saves a summary of the runs")
      ("threshold", po::value<std::string>()->notifier(&ui::threshold),
       "sets the success threshold for a run");

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
      std::cout << vita_sr_version1 << '\n' << vita_sr_version2 << '\n';
      return -1;
    }

    if (vm.count("help"))
      return -1;
  }
  catch(std::exception &e)
  {
    std::cerr << vita::k_s_error << ' ' << e.what() << '\n';
    return 0;
  }

  return 1;
}

int main(int argc, char *const argv[])
{
  std::cout.setf(std::ios::boolalpha);
  std::cout << ui::header << '\n';

  vita::src_problem problem;
  ui::problem = &problem;

  const int ret(parse_command_line(argc, argv));
  if (ret < 0)  // help or version option
    return EXIT_SUCCESS;

  if (ret == 0)  // error
    return EXIT_FAILURE;

  assert(ret > 0);

  if (problem.data()->size())
    ui::go();
  else
  {
    // Do not change the name of the variable:
    //   po::options_description interface("Interface");
    // don't compile with mingw.
    po::options_description interf("Interface");
    interf.add_options()
      ("go", po::value<bool>()->zero_tokens()->notifier(&ui::go),
       "let's go!")
      ("environment",
       po::value<bool>()->zero_tokens()->notifier(&ui::environment),
       "show the environment")
      ("exit", po::value<bool>()->zero_tokens()->notifier(&ui::exit),
       "quit the program");
    ui::cmdl_opt.add(interf);

    vita::cli::command_line_interpreter cli(ui::cmdl_opt, "> ");
    cli.interpret(std::cin);
  }

  return EXIT_SUCCESS;
}
