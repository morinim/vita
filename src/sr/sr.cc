/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
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
  "Vita - Symbolic Regression and classification v0.9.12"
  );
const std::string vita_sr_version2(
  "Copyright 2011-2016 EOS di Manlio Morini (http://www.eosdev.it/)"
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
  using vita::print;
  using vita::trilean;
  using vita::fitness_t;

  vita::environment &env(problem->env);

  if (env.code_length && env.code_length <= problem->categories())
  {
    const decltype(env.code_length) new_length(2 * problem->categories());
    print.warning("Adjusting code length (", env.code_length, " => ",
                  new_length, ")");
    env.code_length = new_length;
  }

  if (env.dss != trilean::no && problem->data()->size() <= 30)
  {
    print.warning("Adjusting DSS (=> false)");
    env.dss = trilean::no;
  }

  if (env.tournament_size)
  {
    if (env.tournament_size < 2)
    {
      print.warning("Adjusting tournament size (=> 2)");
      env.tournament_size = 2;
    }

    if (env.mate_zone && env.tournament_size > *env.mate_zone)
    {
      print.warning("Adjusting tournament size (", env.tournament_size, " => ",
                    *env.mate_zone, ")");
      env.tournament_size = *env.mate_zone;
    }

    if (env.individuals && env.tournament_size > env.individuals)
    {
      print.warning("Adjusting tournament size (", env.tournament_size, " => ",
                    env.individuals, ")");
      env.tournament_size = env.individuals;
    }
  }

  if (!env.threshold.fitness.size() && env.threshold.accuracy < 0.0)
  {
    if (problem->classification())
    {
      env.threshold.fitness = fitness_t(1, vita::copies_of);
      env.threshold.accuracy = 0.99;

      print.info("Accuracy threshold set to ", env.threshold.accuracy);
    }
    else  // symbolic regression
    {
      env.threshold.fitness = {-0.0001};

      print.info("Fitness threshold set to ", env.threshold.fitness);
    }
  }
}

///
/// \param[in] s a value to be interpreted as boolean.
/// \return `true` if `s` is not zero and not `"false"`.
///
bool is_true(const std::string &s)
{
  return s != "0" && !vita::iequals(s, "false");
}

///
/// Contains the text-based command line interface. The interface is not
/// intented to be human friendly (no code completion, command history,
/// submenu...). We just want a light interface for the Python GUI and a simple
/// way to debug.
///
namespace ui
{
using vita::print;

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
  assign(problem->env.arl, is_true(v));

  print.info("Adaptive Representation through Learning is ", problem->env.arl);
}

///
/// \param[in] size brood size for recombination (0 to disable).
///
/// Sets the brood size for recombination.
///
void brood(unsigned size)
{
  problem->env.brood_recombination = size;

  print.info("Brood size for recombination set to ", size);
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

  print.info("Code length set to ", length);
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
    print.warning("Adjusting crossover probability (", r, " => 0.0)");
  }
  else if (r > 1.0)
  {
    r = 1.0;
    print.warning("Adjusting crossover probability (", r, " => 1.0)");
  }

  problem->env.p_cross = r;

  print.info("Crossover rate set to ", r);
}

///
/// \param[in] data_file guess what? This is the data file to be parsed.
/// \return `true` if data have been correctly read and parsed.
///
bool data(const std::string &data_file)
{
  print.info("Reading data file ", data_file, "...");

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
    print.info("Dataset read. Examples: ", parsed,
               ", categories: ",           problem->categories(),
               ", features: ",             problem->variables(),
               ", classes: ",              problem->classes());
  else
    print.error("Dataset file format error");

  return parsed;
}

///
/// \param[in] v a value for DSS (\see is_true).
///
/// Turn on/off the Dynamic Subset Selection algorithm.
  ///
void dss(const std::string &v)
{
  assign(problem->env.dss, is_true(v));

  print.info("Dynamic Subset Selection is ", problem->env.dss);
}

///
/// \param[in] v a value for elitism (\see is_true).
///
void elitism(const std::string &v)
{
  assign(problem->env.elitism, is_true(v));

  print.info("Elitism is ", problem->env.elitism);
}

///
/// Shows the environment.
///
void environment(bool)
{
  print.info("NOT READY");
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
    print.info("Evaluator is ", keyword,
               (eva_args.empty() ? std::string()
                                 : " (parameters: " + eva_args + ")"));
  }
  else
    print.error("Wrong argument for evaluator command");
}

///
/// Quit program.
///
void exit(bool)
{
  print.output("Bye");
}

///
/// \param[in] g maximum number of generations in a run.
///
/// Sets the maximum number of generations in a run.
///
void generations(unsigned g)
{
  problem->env.generations = g;

  print.info("Generations set to ", g);
}

///
/// Starts the search.
///
void go(bool = true)
{
  if (!problem->data()->size())
  {
    print.error("Missing data set");
    return;
  }

  if (!problem->env.sset->enough_terminals())
  {
    print.error("Too few terminals");
    return;
  }

  fix_parameters(problem);

  vita::src_search<vita::i_mep, vita::alps_es> s(*problem);

  if (eva != vita::evaluator_id::undefined)
    s.set_evaluator(eva, eva_args);

  s.run(runs);
}

///
/// \param[in] g maximum number of generations without improvement in a run
///              (0 to disable).
///
/// Sets the maximum number of generations without improvement in a run.
void gwi(unsigned g)
{
  problem->env.g_without_improvement = g;

  print.info("Max number of generations without improvement set to ", g);
}

///
/// Shows the help screen.
///
void help(bool)
{
  print.output(cmdl_opt);
}

///
/// Shows the version header.
///
void version(bool)
{
  print.output(vita_sr_version1, '\n', vita_sr_version2);
}

///
/// \brief Set mating zone
/// \param[in] z mating zone (0 for panmictic).
///
void mate_zone(unsigned z)
{
  problem->env.mate_zone = z;

  print.info("Mate zone set to ", z);
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
    print.warning("Adjusting mutation probability (", r, " => 0.0)");
  }
  else if (r > 1.0)
  {
    r = 1.0;
    print.warning("Adjusting mutation probability (", r, " => 1.0)");
  }

  problem->env.p_mutation = r;

  print.info("Mutation rate set to ", r);
}

///
/// \param[in] ts The dataset used as test set.
/// \return `true` if data have been correctly read and parsed.
///
bool testset(const std::string &ts)
{
  print.info("Reading test set file ", ts, "...");

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
    print.info("Testset read. Examples: ", parsed,
               ", categories: ",           problem->categories(),
               ", features: ",             problem->variables(),
               ", classes: ",              problem->classes());
  else
    print.error("Test set file format error");

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

  print.info("Tournament size set to ", n);
}

///
/// \param[in] l number of layers of the population
///
/// Sets the number of layers of the population.
///
void layers(unsigned l)
{
  problem->env.layers = l;

  print.info("Number of layers set to ",
             (l ? std::to_string(l) : "automatic"));
}

///
/// \param[in] size number of individuals in a layer of the population.
///
/// Sets the number of individuals in a layer of the population.
///
void population_size(unsigned size)
{
  problem->env.individuals = size;

  print.info("Population size set to ",
             (size ? std::to_string(size) : "automatic"));
}

///
/// \param[in] seed sets the seed for the pseudo-random number generator.
///                 Pseudo-random sequences are repeatable by using the same
///                 seed value.
///
void random_seed(unsigned seed)
{
  vita::random::seed(seed);

  print.info("Random seed is ", seed);
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

  print.info("Number of runs set to ", r);
}

///
/// \param[in] v if `true` saves the list of active ADFs.
///
void stat_arl(const std::string &v)
{
  problem->env.stat.arl = is_true(v);

  print.info("ARL is ", v);
}

///
/// \param[in] dir a directory.
///
/// Logs statistics in the `stat.dir` directory.
///
void stat_dir(const std::string &dir)
{
  problem->env.stat.dir = dir;

  print.info("Logging folder is ", dir);
}

///
/// \param[in] v should we save the dynamic execution status file?
///
void stat_dynamic(const std::string &v)
{
  problem->env.stat.dynamic = is_true(v);

  print.info("Dynamic evolution logging is ", problem->env.stat.dynamic);
}

///
/// \param[in] v should we save the layers status file?
///
void stat_layers(const std::string &v)
{
  problem->env.stat.layers = is_true(v);

  print.info("Layers logging is ", problem->env.stat.layers);
}

///
/// \param[in] v should we save the population status file?
///
void stat_population(const std::string &v)
{
  problem->env.stat.population = is_true(v);

  print.info("Population logging is ", problem->env.stat.population);
}

///
/// \param[in] v if `true` saves a summary of the runs.
///
void stat_summary(const std::string &v)
{
  problem->env.stat.summary = true;

  print.info("Summary logging is ", v);
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
    print.info("Using default symbol set");
    problem->setup_default_symbols();
  }
  else
  {
    print.info("Reading symbol file ", symbol_file, "...");

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
      print.error("Symbol file format error");
      return false;
    }

    print.info("Symbolset read. Symbols: ", parsed);
  }

  if (!problem->env.sset->enough_terminals())
  {
    print.error("Too few terminals");
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

  if (set)
    print.info("Threshold is ", v);
  else
    print.error("Invalid threshold value");
}

///
/// \param[in] bits number of bits used for the cache (which contains `2^bits`
///                 elements).
///
void cache(unsigned bits)
{
  Expects(bits);
  problem->env.cache_size = bits;

  print.info("Cache size is ", bits, " bits");
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

  if (set)
    print.info("Validation set percentage is ", v);
  else
    print.error("Invalid validation percentage");
}

///
/// \param[in] v verbosity level.
///
void verbosity(unsigned v)
{
  if (v > vita::log::L_ALL)
    v = vita::log::L_ALL;

  print.verbosity(static_cast<vita::log::level>(v));

  print.info("Verbosity is ", v);
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
       po::value<unsigned>()->zero_tokens()->implicit_value(vita::log::L_INFO)
       ->notifier(&ui::verbosity),
       "set verbose level")
      ("debug",
       po::value<unsigned>()->zero_tokens()->implicit_value(vita::log::L_DEBUG)
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
       "turns on/off the Dynamic Subset Selection algorithm")
      ("generations,g", po::value<unsigned>()->notifier(&ui::generations),
       "sets the maximum number of generations in a run")
      ("gwi", po::value<unsigned>()->notifier(&ui::gwi),
       "sets the maximum number of generations without improvement in a run "
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
      return -1;

    if (vm.count("help"))
      return -1;
  }
  catch(std::exception &e)
  {
    vita::print.error(e.what());
    return 0;
  }

  return 1;
}

int main(int argc, char *const argv[])
{
  std::cout.setf(std::ios::boolalpha);
  vita::print.output(ui::header);

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
