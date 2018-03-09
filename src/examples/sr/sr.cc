/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
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

#include <boost/program_options.hpp>

#include "kernel/environment.h"
#include "kernel/log.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/problem.h"
#include "kernel/src/primitive/factory.h"
#include "kernel/src/search.h"

#include "command_line_interpreter.h"

namespace po = boost::program_options;

namespace
{
const std::string vita_sr_version1(
  "Vita - Symbolic Regression and classification"
  );
const std::string vita_sr_version2(
  "Copyright 2011-2017 EOS di Manlio Morini (http://eosdev.it/)"
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
void fix_parameters(vita::src_problem *problem)
{
  using vita::log;
  using vita::trilean;
  using vita::fitness_t;
  using vita::with_size;

  vita::environment &env(problem->env);

  if (env.mep.code_length && env.mep.code_length <= problem->categories())
  {
    const decltype(env.mep.code_length) new_length(2 * problem->categories());
    vitaWARNING << "Adjusting code length (" << env.mep.code_length
                << " => " << new_length << ')';
    env.mep.code_length = new_length;
  }

  if (env.dss.value_or(0) > 0 && problem->data()->size() <= 30)
  {
    vitaWARNING << "Adjusting DSS (=> disabled)";
    env.dss = 0;
  }

  if (env.tournament_size)
  {
    if (env.tournament_size < 2)
    {
      vitaWARNING << "Adjusting tournament size (=> 2)";
      env.tournament_size = 2;
    }

    if (env.mate_zone && env.tournament_size > env.mate_zone)
    {
      vitaWARNING << "Adjusting tournament size ("
                  << env.tournament_size << " => " << env.mate_zone << ')';
      env.tournament_size = env.mate_zone;
    }

    if (env.individuals && env.tournament_size > env.individuals)
    {
      vitaWARNING << "Adjusting tournament size ("
                  << env.tournament_size << " => " << env.individuals  << ')';
      env.tournament_size = env.individuals;
    }
  }

  if (!env.threshold.fitness.size() && env.threshold.accuracy < 0.0)
  {
    if (problem->classification())
    {
      env.threshold.fitness = fitness_t(with_size(1));
      env.threshold.accuracy = 0.99;

      vitaINFO << "Accuracy threshold set to " << env.threshold.accuracy;
    }
    else  // symbolic regression
    {
      env.threshold.fitness = {-0.0001};

      vitaINFO << "Fitness threshold set to " << env.threshold.fitness;
    }
  }
}

///
/// \param[in] s a value to be interpreted as boolean
/// \return      `true` if `s` is not zero and not `"false"`
///
bool is_true(const std::string &s)
{
  return s != "0" && !vita::iequals(s, "false");
}

///
/// Contains the text-based command line interface.
///
/// The interface is not intented to be human friendly (no code completion,
/// command history, submenu...). We just want a light interface for the
/// Python GUI and a simple way of debugging.
///
namespace ui
{
using vita::log;

const std::string header(
  "(==(     )==)\n"
  " `-.`. ,',-' \n"
  "    _,-'       " + vita_sr_version1 + "\n" \
  " ,-',' `.`-.   " + vita_sr_version2 + "\n" \
  "(==(     )==)\n"
  " `-.`. ,',-'   " + vita_sr_defs + "\n"     \
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
/// Sets Adaptive Representation through Learning.
///
/// \param[in] v a value for ARL (\see is_true)
///
void arl(const std::string &v)
{
  assign(problem->env.arl, is_true(v));

  vitaINFO << "Adaptive Representation through Learning is "
           << problem->env.arl;
}

///
/// Sets the brood size for recombination.
///
/// \param[in] size brood size for recombination (0 to disable)
///
void brood(unsigned size)
{
  problem->env.brood_recombination = size;

  vitaINFO << "Brood size for recombination set to " << size;
}

///
/// Sets the code/genome length of an individual.
///
/// \param[in] length code/genome length of an individual
///
void code_length(unsigned length)
{
  Expects(length);

  problem->env.mep.code_length = length;

  vitaINFO << "Code length set to " << length;
}

///
/// Sets the overall probability that crossover will occour between winners
/// in a tournament.
///
/// \param[in] r range is `[0,1]`
///
void crossover_rate(double r)
{
  if (r < 0.0)
  {
    r = 0.0;
    vitaWARNING << "Adjusting crossover probability (" << r << " => 0.0)";
  }
  else if (r > 1.0)
  {
    r = 1.0;
    vitaWARNING << "Adjusting crossover probability (" << r << " => 1.0)";
  }

  problem->env.p_cross = r;

  vitaINFO << "Crossover rate set to " << r;
}

///
/// \param[in] data_file guess what? This is the data file to be parsed
/// \return              `true` if data have been correctly read and parsed
///
bool data(const std::string &data_file)
{
  vitaINFO << "Reading data file " << data_file << "...";

  std::size_t parsed(0);
  try
  {
    parsed = problem->load(data_file).first;
  }
  catch(...)
  {
    parsed = 0;
  }

  if (parsed)
    vitaINFO << "Dataset read. Examples: " << parsed
             << ", categories: " << problem->categories()
             << ", features: " << problem->variables()
             << ", classes: " << problem->classes();
  else
    vitaERROR << "Dataset file format error";

  return parsed;
}

///
/// Turn on/off the Dynamic Subset Selection algorithm.
///
/// \param[in] s a value for DSS
///
void dss(const std::string &s)
{
  problem->env.dss = decltype(problem->env.dss){s};

  if (problem->env.dss.has_value())
    vitaINFO << "Dynamic Subset Selection is " << *problem->env.dss;
  else
    vitaINFO << "Auto-tuning the parameter";
}

///
/// Sets the elitism tendency.
///
/// \param[in] v a value for elitism
///
void elitism(const std::string &v)
{
  assign(problem->env.elitism, is_true(v));

  vitaINFO << "Elitism is " << problem->env.elitism;
}

///
/// Shows the environment.
///
void environment(bool)
{
  vitaINFO << "NOT IMPLEMENTED";
}

///
/// param[in] v the chosen evaluator
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
    vitaINFO << "Evaluator is " << keyword;
    if (!eva_args.empty())
    {
      vitaINFO << " (parameters: " << eva_args << ')';
    }
  }
  else
    vitaERROR << "Wrong argument for evaluator command";
}

///
/// Quit program.
///
void exit(bool)
{
  vitaOUTPUT << "Bye";
}

///
/// Sets the maximum number of generations in a run.
///
/// \param[in] g maximum number of generations in a run
///
void generations(unsigned g)
{
  problem->env.generations = g;

  vitaINFO << "Generations set to " << g;
}

///
/// Starts the search.
///
void go(bool = true)
{
  if (!problem->data()->size())
  {
    vitaERROR << "Missing data set";
    return;
  }

  if (!problem->sset.enough_terminals())
  {
    vitaERROR << "Too few terminals";
    return;
  }

  fix_parameters(problem);

  vita::src_search<vita::i_mep, vita::alps_es> s(*problem);

  if (eva != vita::evaluator_id::undefined)
    s.set_evaluator(eva, eva_args);

  s.run(runs);
}

///
///
/// Sets the maximum number of generations without improvement in a run.
///
/// \param[in] g maximum number of generations without improvement in a run
///              (`0` for auto-tuning, large number for disabling)
void max_stuck_time(unsigned g)
{
  problem->env.max_stuck_time = g;

  if (problem->env.max_stuck_time.has_value())
    vitaINFO << "Max number of generations without improvement set to " << g;
  else
    vitaINFO << "Auto-tuning the parameter";
}

///
/// Shows the help screen.
///
void help(bool)
{
  vitaOUTPUT << cmdl_opt;
}

///
/// Shows the version header.
///
void version(bool)
{
  vitaOUTPUT << vita_sr_version1 << '\n' << vita_sr_version2;
}

///
/// Sets mating zone.
///
/// \param[in] z mating zone (`0` for panmictic)
///
void mate_zone(unsigned z)
{
  problem->env.mate_zone = z;

  vitaINFO << "Mate zone set to " << z;
}

///
/// Sets the overall probability of mutation of the individuals that have
/// been selected as winners in a tournament.
///
/// \param[in] r mutation rate, range is `[0,1]`
///
void mutation_rate(double r)
{
  if (r < 0.0)
  {
    r = 0.0;
    vitaWARNING << "Adjusting mutation probability (" << r << " => 0.0)";
  }
  else if (r > 1.0)
  {
    r = 1.0;
    vitaWARNING << "Adjusting mutation probability (" << r << " => 1.0)";
  }

  problem->env.p_mutation = r;

  vitaINFO << "Mutation rate set to " << r;
}

///
/// \param[in] ts the dataset used as test set
/// \return       `true` if data have been correctly read and parsed
///
bool testset(const std::string &ts)
{
  vitaINFO << "Reading test set file " << ts << "...";

  std::size_t parsed(0);
  try
  {
    parsed = problem->load_test_set(ts);
  }
  catch(...)
  {
    parsed = 0;
  }

  if (parsed)
  {
    vitaINFO << "Testset read. Examples: " << parsed
             << ", categories: " << problem->categories()
             << ", features: " << problem->variables()
             << ", classes: " << problem->classes();
  }
  else
  {
    vitaERROR << "Test set file format error";
  }

  return parsed;
}

///
/// Sets the number of individuals examined for choosing parents.
///
/// \param[in] n number of individuals
///
void tournament_size(unsigned n)
{
  problem->env.tournament_size = n;

  vitaINFO << "Tournament size set to " << n;
}

///
/// Sets the number of layers of the population.
///
/// \param[in] l number of layers of the population
///
void layers(unsigned l)
{
  problem->env.layers = l;

  vitaINFO << "Number of layers set to "
           << (l ? std::to_string(l) : "automatic");
}

///
/// Sets the number of individuals in a layer of the population.
///
/// \param[in] size number of individuals in a layer of the population
///
void population_size(unsigned size)
{
  problem->env.individuals = size;

  vitaINFO << "Population size set to "
           << (size ? std::to_string(size) : "automatic");
}

///
/// \param[in] seed sets the seed for the pseudo-random number generator.
///                 Pseudo-random sequences are repeatable by using the same
///                 seed value
///
void random_seed(unsigned seed)
{
  vita::random::seed(seed);

  vitaINFO << "Random seed is " << seed;
}

///
/// Number of runs to be tried.
///
/// \param[in] r number of runs
///
void set_runs(unsigned r)
{
  assert(r);
  runs = r;

  vitaINFO << "Number of runs set to " << r;
}

///
/// \param[in] v if `true` saves the list of active ADFs
///
void stat_arl(const std::string &v)
{
  const bool b(is_true(v));
  problem->env.stat.arl_file = b ? "arl" : "";
  vitaINFO << "ARL logging is " << b;
}

///
/// Number of runs to be tried.
///
/// \param[in] dir a directory
///
void stat_dir(const std::string &dir)
{
  problem->env.stat.dir = dir;

  vitaINFO << "Logging folder is " << dir;
}

///
/// \param[in] v should we save the dynamic execution status file?
///
void stat_dynamic(const std::string &v)
{
  const bool b(is_true(v));
  problem->env.stat.dynamic_file = b ? "dynamic" : "";

  vitaINFO << "Dynamic evolution logging is " << b;
}

///
/// \param[in] v should we save the layers status file?
///
void stat_layers(const std::string &v)
{
  const bool b(is_true(v));
  problem->env.stat.layers_file = b ? "layers" : "";

  vitaINFO << "Layers logging is " << b;
}

///
/// \param[in] v should we save the population status file?
///
void stat_population(const std::string &v)
{
  const bool b(is_true(v));
  problem->env.stat.population_file = b ? "population" : "";

  vitaINFO << "Population logging is " << b;
}

///
/// \param[in] v if `true` saves a summary of the runs
///
void stat_summary(const std::string &v)
{
  const bool b(is_true(v));
  problem->env.stat.summary_file = b ? "summary" : "";

  vitaINFO << "Summary logging is " << b;
}

///
/// \param[in] symbol_file file containing a list of symbols (functions and
///                        terminals). If `symbol_file` is `empty()`, the
///                        default symbol set will be loaded
/// \return                `true` if symbols have been correctly read and
///                        parsed
///
bool symbols(const std::string &symbol_file)
{
  if (symbol_file.empty())
  {
    vitaINFO << "Using default symbol set";
    problem->setup_default_symbols();
  }
  else
  {
    vitaINFO << "Reading symbol file " << symbol_file << "...";

    std::size_t parsed(0);
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
      vitaERROR << "Symbol file format error";
      return false;
    }

    vitaINFO << "Symbolset read. Symbols: " << parsed;
  }

  if (!problem->sset.enough_terminals())
  {
    vitaERROR << "Too few terminals";
    return false;
  }

  return true;
}

///
/// Sets the success threashold.
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

  if (set)
    vitaINFO << "Threshold is " << v;
  else
    vitaERROR << "Invalid threshold value";
}

///
/// \param[in] bits number of bits used for the cache (which contains `2^bits`
///                 elements)
///
void cache(unsigned bits)
{
  Expects(bits);
  problem->env.cache_size = bits;

  vitaINFO << "Cache size is " << bits << " bits";
}

///
/// Sets percent of the dataset used for validation.
///
/// \param[in] v range is `[0,1]` or `[0%,100%]`
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

  if (set)
    vitaINFO << "Validation set percentage is " << v;
  else
    vitaERROR << "Invalid validation percentage";
}

///
/// \param[in] v verbosity level
///
void verbosity(unsigned v)
{
  if (v > vita::log::ALL)
    v = vita::log::ALL;

  vita::log::reporting_level = static_cast<vita::log::level>(v);

  vitaINFO << "Verbosity is " << v;
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
      ("version,v", po::value<bool>()->zero_tokens()->notifier(&ui::version),
       "print version string")
      ("help,h", po::value<bool>()->zero_tokens()->notifier(&ui::help),
       "produce the help message")
      ("quiet",
       po::value<unsigned>()->zero_tokens()->implicit_value(0)->notifier(
         &ui::verbosity),
       "turn off verbosity")
      ("verbose",
       po::value<unsigned>()->zero_tokens()->implicit_value(vita::log::INFO)
       ->notifier(&ui::verbosity),
       "set verbose level")
      ("debug",
       po::value<unsigned>()->zero_tokens()->implicit_value(vita::log::DEBUG)
       ->notifier(&ui::verbosity),
       "print debug information");

    po::options_description data("Data");
    data.add_options()
      ("data,d", po::value<std::string>()->notifier(&ui::data), "data set")
      ("symbols,s",
       po::value<std::string>()->notifier(&ui::symbols),
       "symbols file")
      ("testset,t", po::value<std::string>()->notifier(&ui::testset),
       "test set")
      ("validation", po::value<std::string>()->notifier(&ui::validation),
       "sets the percent of the dataset used for validation");

    po::options_description config("Config");
    config.add_options()
      ("cache", po::value<unsigned>()->notifier(&ui::cache),
       "number of bits used for the cache (cache contains `2^bits` elements)")
      ("evaluator", po::value<std::string>()->notifier(&ui::evaluator),
       "sets preferred evaluator "
       "(count, mae, rmae, mse, binary, dynslot, gaussian)")
      ("random-seed", po::value<unsigned>()->notifier(&ui::random_seed),
       "sets the seed for the pseudo-random number generator. "
       "Pseudo-random sequences are repeatable by using the same seed value");

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
       "sets the overall probability of mutation of the individuals that have "
       "been selected as winners in a tournament. Range is [0,1]")
      ("crossover-rate,c", po::value<double>()->notifier(&ui::crossover_rate),
       "sets the overall probability that crossover will occour between "
       "winners in a tournament. Range is [0,1]")
      ("tournament-size",
       po::value<unsigned>()->notifier(&ui::tournament_size),
       "number of individuals examined for choosing parents")
      ("brood", po::value<unsigned>()->notifier(&ui::brood),
       "sets the brood size for recombination (0 to disable)")
      ("dss", po::value<std::string>()->notifier(&ui::dss),
       "controls the Dynamic Subset Selection algorithm")
      ("generations,g", po::value<unsigned>()->notifier(&ui::generations),
       "sets the maximum number of generations in a run")
      ("max-stuck-time", po::value<unsigned>()->notifier(&ui::max_stuck_time),
       "sets the maximum number of generations without improvement in a run "
       "(large number for disabling, 0 for auto-tuning)")
      ("runs,r", po::value<unsigned>()->notifier(&ui::set_runs),
       "number of runs to be tried")
      ("mate-zone", po::value<unsigned>()->notifier(&ui::mate_zone),
       "mating zone (0 for panmictic)")
      ("arl", po::value<std::string>()->notifier(&ui::arl),
       "adaptive Representation through Learning");

    po::options_description statistics("Statistics");
    statistics.add_options()
      ("stat-dir", po::value<std::string>()->notifier(&ui::stat_dir),
       "base path for log files")
      ("stat-arl", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_arl),
       "set ARL logging status")
      ("stat-dynamic", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_dynamic),
       "set general real-time logging status")
      ("stat-layers", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_layers),
       "set layer-specific information logging status")
      ("stat-population", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_population),
       "set population-specific information logging status")
      ("stat-summary", po::value<std::string>()->implicit_value("true")->notifier(&ui::stat_summary),
       "set end-of-run summary logging status")
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
      return -1;

    if (vm.count("help"))
      return -1;
  }
  catch(std::exception &e)
  {
    using vita::log;
    vitaERROR << e.what();
    return 0;
  }

  return 1;
}

int main(int argc, char *const argv[])
{
  using vita::log;

  std::cout.setf(std::ios::boolalpha);
  vitaOUTPUT << ui::header;

  vita::src_problem problem;
  ui::problem = &problem;

  const int ret(parse_command_line(argc, argv));
  if (ret < 0)  // help or version option
    return EXIT_SUCCESS;

  if (ret == 0)  // error
    return EXIT_FAILURE;

  if (problem.data()->size())
    ui::go();
  else
  {
    // Do not change the name of the variable: e.g.
    // `po::options_description interface("Interface");`
    // won't compile with mingw.
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
