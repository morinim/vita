/**
 *
 *  \file sr.cc
 *
 *  \author Manlio Morini
 *  \date 2011/03/19
 *
 *  This file is part of VITA
 *
 */

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "environment.h"
#include "search.h"
#include "src_problem.h"

const std::string vita_sr_version("Vita v1.02 - Symbolic Regression and classification\nCopyright EOS Development (http://www.eosdev.it)");

bool verbose(false);

unsigned runs(1);

vita::src_problem problem;

void setup_default_symbols()
{  
  problem.env.insert(new vita::sr::constant(1));
  problem.env.insert(new vita::sr::constant(2));
  problem.env.insert(new vita::sr::constant(3));
  problem.env.insert(new vita::sr::constant(4));
  problem.env.insert(new vita::sr::constant(5));
  problem.env.insert(new vita::sr::constant(6));
  problem.env.insert(new vita::sr::constant(7));
  problem.env.insert(new vita::sr::constant(8));
  problem.env.insert(new vita::sr::constant(9));
  
  problem.env.insert(new vita::sr::abs());
  problem.env.insert(new vita::sr::add());
  problem.env.insert(new vita::sr::div());
  problem.env.insert(new vita::sr::ln());
  problem.env.insert(new vita::sr::mul());
  problem.env.insert(new vita::sr::mod());
  problem.env.insert(new vita::sr::sub());
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
      ("version,v","print version string.")
      ("help,h","produces help message.")
      ("verbose",po::bool_switch(&verbose),"shows more details.");
    
    po::options_description data("Data");
    data.add_options()
      ("data,d",po::value(&data_file),"Data file.")
      ("symbols,s",po::value(&symbols_file),"Symbols file.");

    po::options_description config("Config");
    config.add_options()
      ("ttable",
       po::value(&problem.env.ttable_size)->default_value(problem.env.ttable_size),
       "Number of bits used for the ttable (ttable contains 2^bit elements).")
      ("random-seed",
       po::value(&random_seed),
       "Sets the seed for the pseudo-random number generator. Pseudo-random sequences are repeatable by using the same seed value.");

    // Declare a group of options that will be allowed both on command line 
    // and in config file.
    po::options_description evolution("Evolution");
    evolution.add_options()
      ("population-size,P", 
       po::value(&problem.env.individuals),
       "Sets the number of programs/individuals in the population.")
      ("program-size,p",
       po::value(&problem.env.code_length),
       "Sets the maximum length of an evolved program in the population.")
      ("mutation-rate,m",
       po::value(&problem.env.p_mutation),
       "Sets the overall probability of mutation of the individuals that have been selected as winners in a tournament. Range is [0,1].")
      ("crossover-rate,c",
       po::value(&problem.env.p_cross),
       "Sets the overall probability that crossover will occour between two winners in a tournament. Range is [0,1].")
      ("parent-tournament,T",
       po::value(&problem.env.par_tournament),
       "Number of individuals chosen at random from the population to identify a parent.") 
      ("g-since-start,g",
       po::value(&problem.env.g_since_start),
       "Sets the maximum number of generations in a run.")
      ("gwi",
       po::value(&problem.env.g_without_improvement),
       "Sets the maximum number of generations without improvement in a run.")
      ("runs,r",
       po::value(&runs),
       "Number of runs to be tried.")
      ("mate-zone",
       po::value(&problem.env.mate_zone),
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
        
    po::options_description cmdl_opt(vita_sr_version+"\n\nsr [options] data_file\n\nAllowed options");
    cmdl_opt.add(generic).add(data).add(config).add(evolution).add(statistics);
       
    po::positional_options_description p;
    p.add("data",-1);
        
    po::variables_map vm;

    store(po::command_line_parser(argc,argv).
          options(cmdl_opt).positional(p).run(),vm);

    notify(vm);

    if (vm.count("version"))
    {
      std::cout << vita_sr_version << std::endl;
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
		<< problem.env.stat_dir	<< std::endl;
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return false;
  }    

  if (verbose) std::cout << "Reading data file (" << data_file << ")... ";
  if (!problem.load_data(data_file))
    return false;
  if (verbose)
  { 
    std::cout << "ok" << std::endl
	      << "  (" << problem.variables() << " variables, " 
	      << problem.classes() << " classes)" << std::endl;
  }
  

  if (symbols_file.empty())
  { 
    if (verbose) std::cout << "  (default symbol set)" << std::endl;
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
bool
run()
{
  vita::search s(problem);

  s.run(verbose,runs);

  // vita::interpreter agent(ind);

  /*
  std::map<number,vita::distribution<vita::fitness_t> > classes;
  for (vita::data<number>::const_iterator t(data.begin());
       t != data.end();
       ++t) 
  {
    for (unsigned i(0); i < vars.size(); ++i)
      vars[i]->val = (*t)[i];

    const vita::fitness_t res(agent.run());

    const number c((*t)[vars.size()]);

    classes[c].add(res);
  }    

  typedef std::map<number,vita::distribution<vita::fitness_t> >::const_iterator const_iterator;
    for (const_iterator i(classes.begin()); i != classes.end(); ++i)
      std::cout << i->first << "    "
		<< i->second.mean << "    " << i->second.variance << "    "
		<< i->second.min << "    " << i->second.max << std::endl;
  */
  /*
  for (vita::data<number>::const_iterator t(data.begin());
       t != data.end();
       ++t)
  {
    for (unsigned i(0); i < vars.size(); ++i)
      vars[i]->val = (*t)[i];

    const vita::fitness_t res(agent.run());
  
    std::cout << res << std::endl;

    number best_class;
    for (const_iterator i(classes.begin()); i != classes.end(); ++i)
      if (std::fabs(res-classes[i->first].mean) < min)
      {
	min = std::fabs(res-classes[i->first].mean);
	best_class = i->first;
      }
			
    if (best_class == (*t)[vars.size()])
      f += 1.0 - ind.eff_size()/1200.0;
  }  
  */

  return true;
}

int main(int argc, char *argv[])
{
  if (!parse_command_line(argc,argv))
    return EXIT_FAILURE;

  run();

  return EXIT_SUCCESS;
}
