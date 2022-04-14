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

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include "kernel/vita.h"
#include "third_party/docopt/docopt.h"

const char USAGE[] =
R"(Vita - Symbolic Regression and classification
Copyright 2011-2020 EOS di Manlio Morini (https://eosdev.it/)

(==(     )==)
 `-.`. ,',-'
    _,-'
 ,-',' `.`-.
(==(     )==)
 `-.`. ,',-'
    _,-'`
 ,-',' `.`-.
(==(     )==)

Usage:
  sr [options] DATASET
  sr -h | --help
  sr -v | --version

Arguments:
  DATASET  filepath of the training set

Options:
  -h --help              shows this screen and exit
  -v --version           shows version and exit
  --quiet                turns off verbosity
  --verbose              turns on information messages
  --debug                prints debug information
  --symbols=SYMBOLS      file specifying symbols used to solve the task
  --validation=<perc>    sets the percent of the dataset used for validation
  --evaluator=<eval>     sets the preferred evaluator
                         (count, mae, rmae, mse, binary, dynslot, gaussian)
  --population=<size>    number of individuals in a layer of the population
  --layers=<layers>      number of layers of the population
  --length=<length>      sets the size of the genome
  --no-elitism           an individual can replace a better one
  --mutation=<rate>      sets the overall probability of mutation of the
                         individuals that have been selected as winners in a
                         tournament. Range is [0,1]
  --crossover=<rate>     sets the overall probability that crossover will
                         occour between winners in a tournament. Range is [0,1]
  --tournament=<size>    number of individuals examined for parents' selection
  --brood=<size>         sets the brood size for recombination (0 to disable)
  --dss=<period>         controls the Dynamic Subset Selection algorithm
  --generations=<gen>    sets the maximum number of generations in a run
  --max-stuck-time=<st>  sets the maximum number of generations without
                         improvement in a run
  --runs=<runs>          number of runs to be tried
  --mate-zone=<dist>     mating zone (0 for panmictic)
  --threshold=<val>      success threshold for a run
  --cache=<bits>         cache will contain `2^bits` elements
  --random-seed=<seed>   sets the seed for the pseudo-random number generator
                         (equences are repeatable by using the same seed value)
  --stat-dir=DIR         base path for log files
  --stat-dynamic         enables real-time logging
  --stat-layers          enables layer-specific information logging
  --stat-population      enables population-specific information logging
  --stat-summary         enables end-of-run summary logging
)";

using args_t = std::map<std::string, docopt::value>;


// Fixes conflicting parameters.
void fix_parameters(vita::src_problem *problem)
{
  using vita::log;
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

// Given a string containing:
// - the representation of a real number in the `[0;1]` range (e.g. "0.5") or
// - a percentage (e.g. "23%")
// returns the corresponding double number in the `[0;100]` range.
double to_percentage(const std::string &v)
{
  return v.back() == '%'
    ? std::stod(v.substr(0, v.length() - 1))
    : std::stod(v) * 100.0;
}

// Contains the text-based command line interface.
//
// The interface is not intented to be human friendly (no code completion,
// command history, submenu...). We just want a light interface for the
// Python GUI and a simple way of debugging.
namespace ui
{
using vita::log;

// Number of runs to be tried.
unsigned runs(1);

// Active evaluator.
vita::evaluator_id eva(vita::evaluator_id::undefined);
std::string eva_args;

// Active validation strategy.
vita::validator_id validator(vita::validator_id::undefined);

// Reference problem (the problem we will work on).
vita::src_problem *problem;

// Sets the brood size for recombination.
//
// `0` to disable brood recombination.
void brood(const args_t &a)
{
  const auto value(a.at("--brood"));
  if (!value)
    return;

  const auto size(value.asLong());
  problem->env.brood_recombination = size;

  if (size)
  {
    vitaINFO << "Brood size for recombination set to " << size;
  }
  else
  {
    vitaINFO << "Brood recombination disabled";
  }
}

// Sets the code/genome length of an individual.
void code_length(const args_t &a)
{
  const auto value(a.at("--length"));
  if (!value)
    return;

  const auto length(value.asLong());
  if (!length)
  {
    vitaWARNING << "Wrong code length. Using default value";
    return;
  }

  problem->env.mep.code_length = length;
  vitaINFO << "Code length set to " << length;
}

// Sets the overall probability that crossover will occour between winners
// in a tournament.
//
// Range is `[0,1]` (also accepted "23%").
void crossover_rate(const args_t &a)
{
  const auto value(a.at("--crossover"));
  if (!value)
    return;

  const double r(to_percentage(value.asString()) / 100.0);
  if (r < 0.0 || r > 1.0)
  {
    vitaWARNING << "Wrong crossover probability. Using default value";
    return;
  }

  problem->env.p_cross = r;
  vitaINFO << "Crossover rate set to " << problem->env.p_cross;
}

// Loads the training set.
void data(const args_t &a)
{
  const auto data_file(a.at("DATASET").asString());
  vitaINFO << "Reading dataset " << data_file << "...";

  const auto parsed(problem->data().read(data_file));

  if (parsed)
    vitaINFO << "...dataset read. Examples: " << parsed
             << ", categories: " << problem->categories()
             << ", features: " << problem->variables()
             << ", classes: " << problem->classes();
  else
    vitaERROR << "Empty dataset";
}

// Turn on/off the Dynamic Subset Selection algorithm.
void dss(const args_t &a)
{
  const auto value(a.at("--dss"));
  if (!value)
    return;

  const auto dss(value.asLong());

  if (dss > 0)
  {
    validator = vita::validator_id::dss;
    problem->env.dss = dss;
    vitaINFO << "Dynamic Subset Selection set to " << dss;
  }
  else
    vitaINFO << "Dynamic Subset Selection disabled";
}

// Disables elitism.
void no_elitism(const args_t &a)
{
  const auto value(a.at("--no-elitism").asBool());
  if (value)
  {
    problem->env.elitism = vita::trilean::no;
    vitaINFO << "Elitism is " << problem->env.elitism;
  }
}

// Selects the active evaluator.
//
// The evaluator must fit the current problem kind (symbolic regression,
// classification) or it won't be used.
void evaluator(const args_t &a)
{
  const auto value(a.at("--evaluator"));
  if (!value)
    return;

  const auto v(value.asString());

  const auto sep(v.find(':'));
  const auto keyword(v.substr(0, sep));

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

// Sets the maximum number of generations in a run.
void generations(const args_t &a)
{
  const auto value(a.at("--generations"));
  if (!value)
    return;

  const auto g(value.asLong());
  if (g == 0)
  {
    vitaWARNING << "Wrong number of generations. Using default value";
    return;
  }

  problem->env.generations = g;
  vitaINFO << "Generations set to " << g;
}

// Starts the search.
void go(bool = true)
{
  Expects(problem->sset.enough_terminals());

  if (!problem->data().size())
  {
    vitaERROR << "Missing data set";
    return;
  }

  fix_parameters(problem);

  vita::src_search<vita::i_mep, vita::alps_es> s(*problem);

  if (eva != vita::evaluator_id::undefined)
    s.evaluator(eva, eva_args);

  if (validator == vita::validator_id::dss
      || validator == vita::validator_id::holdout)
    s.validation_strategy(validator);

  s.run(runs);
}

// Sets the maximum number of generations without improvement in a run.
void max_stuck_time(const args_t &a)
{
  const auto value(a.at("--max-stuck-time"));
  if (!value)
    return;

  const auto g(value.asLong());
  problem->env.max_stuck_time = g;
  if (g)
    vitaINFO << "Max number of generations without improvement set to " << g;
  else
    vitaINFO << "Max number of generations without improvement disabled";
}

// Sets mating zone (`0` for panmictic).
void mate_zone(const args_t &a)
{
  const auto value(a.at("--mate-zone"));
  if (!value)
    return;

  problem->env.mate_zone = value.asLong();
  vitaINFO << "Mate zone set to " << problem->env.mate_zone;
}

// Sets the overall probability of mutation of the individuals that have
// been selected as winners in a tournament.
//
// Range is `[0,1]`
void mutation_rate(const args_t &a)
{
  const auto value(a.at("--mutation"));
  if (!value)
    return;

  const double r(to_percentage(value.asString()) / 100.0);
  if (r < 0.0 || r > 1.0)
  {
    vitaWARNING << "Wrong mutation probability. Using default value";
    return;
  }

  problem->env.p_mutation = r;
  vitaINFO << "Mutation rate set to " << problem->env.p_mutation;
}

// Sets the number of individuals examined for choosing parents.
void tournament_size(const args_t &a)
{
  const auto value(a.at("--tournament"));
  if (!value)
    return;

  const auto n(value.asLong());
  if (n == 0)
  {
    vitaWARNING << "Wrong tournament size. Using default value";
    return;
  }

  problem->env.tournament_size = n;
  vitaINFO << "Tournament size set to " << problem->env.tournament_size;
}

// Sets the number of layers of the population.
void layers(const args_t &a)
{
  const auto value(a.at("--layers"));
  if (!value)
    return;

  const auto l(value.asLong());
  if (!l)
  {
    vitaWARNING << "Wrong number of layers. Default initializing";
    return;
  }

  problem->env.layers = l;
  vitaINFO << "Number of layers set to " << l;
}

// Sets the number of individuals in a layer of the population.
void population_size(const args_t &a)
{
  const auto value(a.at("--population"));
  if (!value)
    return;

  const auto size(value.asLong());
  if (!size)
  {
    vitaWARNING << "Wrong population size. Using default value";
    return;
  }

  problem->env.individuals = size;
  vitaINFO << "Population size sets to " << size;
}

// Sets the seed for the pseudo-random number generator.
//
// Pseudo-random sequences are repeatable by using the same seed value.
void random_seed(const args_t &a)
{
  const auto value(a.at("--random-seed"));
  if (!value)
    return;

  const auto seed(value.asLong());
  vita::random::seed(seed);
  vitaINFO << "Random seed is " << seed;
}

// Number of runs to be tried.
void set_runs(const args_t &a)
{
  const auto value(a.at("--runs"));
  if (!value)
    return;

  const auto r(value.asLong());
  if (!r)
  {
    vitaWARNING << "Ignoring wrong number of runs";
    return;
  }

  runs = r;
  vitaINFO << "Number of runs set to " << r;
}

// Sets the base path for log files.
void stat_dir(const args_t &a)
{
  if (const auto value = a.at("--stat-dir"))
  {
    if (const std::filesystem::path sd = value.asString();
        std::filesystem::is_directory(sd))
    {
      problem->env.stat.dir = sd;
      vitaINFO << "Logging folder is " << problem->env.stat.dir;
    }
    else
      vitaERROR << "Directory doesn't exist (" << value.asString() << ")";
  }
}

/// Sets the dynamic execution status file.
void stat_dynamic(const args_t &a)
{
  if (a.at("--stat-dynamic").asBool())
  {
    problem->env.stat.dynamic_file = "dynamic";
    vitaINFO << "Dynamic evolution logging is enabled";
  }
}

// Sets the layers status file.
void stat_layers(const args_t &a)
{
  if (a.at("--stat-layers").asBool())
  {
    problem->env.stat.layers_file = "layers";
    vitaINFO << "Layers logging is enabled";
  }
}

// Sets the population status file.
void stat_population(const args_t &a)
{
  if (a.at("--stat-population").asBool())
  {
    problem->env.stat.population_file = "population";
    vitaINFO << "Population logging is enabled";
  }
}

// Sets the summary file.
void stat_summary(const args_t &a)
{
  if (a.at("--stat-summary").asBool())
  {
    problem->env.stat.summary_file = "summary";
    vitaINFO << "Summary logging is enabled";
  }
}

// Reads the file containing the symbols (functions and terminals).
void symbols(const args_t &a)
{
  const auto value(a.at("--symbols"));
  const auto symbol_file(value ? value.asString() : std::string());

  try
  {
    // If there isn't a file, generate the default symbol set.
    problem->setup_symbols(symbol_file);
  }
  catch (const vita::exception::data_format &e)
  {
    vitaERROR << e.what();
    std::exit(EXIT_FAILURE);
  }

  if (!problem->sset.enough_terminals())
  {
    vitaERROR << "Too few terminals";
    std::exit(EXIT_FAILURE);
  }
}

// Sets the success threashold.
//
// If the output value of a run is greater than the threshold, it's scored as
// a success.
// When threshold is a:
// - simple number, the reference value is the fitness;
// - a percentage, the reference value is the accuracy.
void threshold(const args_t &a)
{
  const auto value(a.at("--threshold"));
  if (!value)
    return;

  const auto v(value.asString());

  if (v.length())
  {
    if (v.back() == '%')
    {
      const auto accuracy(to_percentage(v));

      if (0.0 < accuracy && accuracy <= 1.0)
      {
        problem->env.threshold.accuracy = accuracy;
        vitaINFO << "Accuracy threshold set to " << v;
      }
    }
    else
    {
      const vita::fitness_t::value_type fitness(std::stod(v));
      problem->env.threshold.fitness = {fitness};
      vitaINFO << "Fitness threshold set to " << v;
    }
  }
  else
    vitaERROR << "Invalid threshold value";
}

// Sets the number of bits used for the cache (`2^bits` elements).
void cache(const args_t &a)
{
  const auto value(a.at("--cache"));
  if (!value)
    return;

  const auto bits(value.asLong());
  if (bits < 10)
  {
    vitaWARNING
      << "Cache too small (11 bits required at least). Using default value";
    return;
  }

  problem->env.cache_size = bits;
  vitaINFO << "Cache size is " << bits << " bits";
}

// Sets percent of the dataset used for validation.
//
// Range is `[0,1]` or `[0%,100%]`.
void validation(const args_t &a)
{
  const auto value(a.at("--validation"));
  if (!value)
    return;

  const auto v(value.asString());
  if (v.length())
  {
    const auto percentage(static_cast<unsigned>(to_percentage(v)));

    if (percentage && percentage <= 90)
    {
      validator = vita::validator_id::holdout;
      problem->env.validation_percentage = percentage;
      vitaINFO << "Validation percentage is " << v;
    }
    else
      vitaERROR << "Invalid validation percentage. Value ignored";
  }
}

// Sets the verbosity level.
void verbosity(const args_t &a)
{
  const auto l(a.at("--debug").asBool() ? vita::log::lALL
               : a.at("--verbose").asBool() ? vita::log::lINFO
               : a.at("--quiet").asBool() ? vita::log::lOFF
               : vita::log::lOUTPUT);

  vita::log::reporting_level = l;
  vitaINFO << "Verbosity is " << l;
}

}  // namespace ui

void parse_command_line(int argc, char *const argv[])
{
  auto args(docopt::docopt(USAGE,
                           {argv + 1, argv + argc},
                           true,      // shows help if requested
                           __DATE__,  // version string
                           false));   // options first (POSIX compliant)

  ui::verbosity(args);

  ui::cache(args);
  ui::evaluator(args);
  ui::random_seed(args);

  ui::population_size(args);
  ui::layers(args);
  ui::code_length(args);
  ui::no_elitism(args);
  ui::mutation_rate(args);
  ui::crossover_rate(args);
  ui::tournament_size(args);
  ui::brood(args);
  ui::dss(args);
  ui::generations(args);
  ui::max_stuck_time(args);
  ui::set_runs(args);
  ui::mate_zone(args);
  ui::threshold(args);

  ui::stat_dir(args);
  ui::stat_dynamic(args);
  ui::stat_layers(args);
  ui::stat_population(args);
  ui::stat_summary(args);

  ui::data(args);
  ui::symbols(args);
  ui::validation(args);
}

int main(int argc, char *const argv[])
{
  using vita::log;

  vita::src_problem problem;
  ui::problem = &problem;

  parse_command_line(argc, argv);

  if (!problem.data().size())
    return EXIT_FAILURE;

  ui::go();

  return EXIT_SUCCESS;
}
