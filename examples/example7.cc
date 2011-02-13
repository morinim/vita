/**
 *
 *  \file example7.cc
 *
 *  \author Manlio Morini
 *  \date 2010/06/11
 *
 *  This file is part of VITA
 *
 *  Building blocks infrastructure test.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "adf.h"
#include "distribution.h"
#include "environment.h"
#include "primitive/sr_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 100;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());
  env.insert(new vita::sr::abs());
  env.insert(new vita::sr::ln());

  vita::distribution<double> individuals, blocks_len, blocks_n, arguments;

  for (unsigned k(0); k < n; ++k)
  {
    vita::individual base;
    unsigned base_es;
    do
    {
      base = vita::individual(env,true);
      base_es = base.eff_size();
    } while (base_es < 5);
      
    individuals.add(base_es);

    std::cout << std::string(40,'-') << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    std::list<unsigned> bl(base.blocks());
    for (std::list<unsigned>::const_iterator i(bl.begin()); i != bl.end(); ++i)
    {
      vita::individual ib(base.get_block(*i));
      vita::individual norm;
      const unsigned first_terminal(ib.normalize(norm));

      if (first_terminal)
      {
        std::vector<unsigned> positions;
        std::vector<vita::symbol_t> types;
        norm.generalize(2,&positions,&types);

        std::cout << std::endl;
        ib.list(std::cout);

        std::cout << "NORMALIZED" << std::endl;
        norm.list(std::cout);

        const unsigned arg_n(positions.size());
        std::cout << std::endl << "Arguments:";
        for (unsigned j(0); j < arg_n; ++j)
          std::cout << " (pos=" << positions[j] << ",type=" << types[j]
                    << ")";
        std::cout << std::endl;
	
        blocks_len.add(ib.eff_size());
        arguments.add(arg_n);
      }
    }
  }

  std::cout << std::string(40,'-') << std::endl
	    << "Individuals effective lengths." << std::endl
	    << "Min: " << individuals.min << "  Mean: " << individuals.mean
	    << "  StdDev: " << std::sqrt(individuals.variance)
	    << "  Max: " << individuals.max << std::endl
	    << "Blocks effective lengths." << std::endl
	    << "Min: " << blocks_len.min << "  Mean: " << blocks_len.mean
	    << "  StdDev: " << std::sqrt(blocks_len.variance)
	    << "  Max: " << blocks_len.max << std::endl
	    << "Number of arguments." << std::endl
	    << "Min: " << arguments.min << "  Mean: " << arguments.mean
	    << "  StdDev: " << std::sqrt(arguments.variance)
	    << "  Max: " << arguments.max << std::endl;
 
  return EXIT_SUCCESS;
}
