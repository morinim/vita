/**
 *
 *  \file example8.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/23
 *
 *  This file is part of VITA
 *
 *  Building blocks run test.
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

  env.code_length = argc > 1 ? atoi(argv[1]) : 5;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());
  env.insert(new vita::sr::abs());
  env.insert(new vita::sr::ln());

  for (unsigned k(0); k < n; ++k)  
  {
    vita::individual base;
    unsigned base_es;
    do
    {
      base = vita::individual(env,true);
      base_es = base.eff_size();
    } while (base_es < 5);

    std::cout << std::string(40,'-') << std::endl << "BASE" << std::endl;
    base.list(std::cout);  
    std::cout << std::endl;
 
    std::list<unsigned> bl(base.blocks());
    for (std::list<unsigned>::const_iterator i(bl.begin()); i != bl.end(); ++i)
    {
      vita::individual blk(base.get_block(*i));
      
      vita::individual norm;
      const unsigned first_terminal(blk.normalize(norm));
      if (first_terminal)
      {
        std::cout << std::endl << "BLOCK at line " << *i << std::endl;
        blk.list(std::cout);
        const boost::any val(vita::interpreter(blk).run());
        if (val.empty())
          std::cout << "Incorrect output.";
        else
          std::cout << "Output: " << boost::any_cast<double>(val);
        std::cout << std::endl;

        std::cout << std::endl << "NORMALIZED" << std::endl;
        norm.list(std::cout);
        const boost::any val_n(vita::interpreter(norm).run());

        if ( val.empty() != val_n.empty() ||
             (!val.empty() && !val_n.empty() &&
              boost::any_cast<double>(val) != boost::any_cast<double>(val_n)) )
        {
          std::cout << "NORMALIZED BLOCK EVAL ERROR." << std::endl;
          return EXIT_FAILURE;
        }
        
        if (norm.eff_size() <= 20)
        {
          vita::individual blk2(norm);
          std::vector<unsigned> positions;
          std::vector<vita::symbol_t> types;
          blk2.generalize(2,&positions,&types);
          vita::adf *const f = new vita::adf(blk2,types,100);
          env.insert(f);
          std::cout << std::endl << f->display() << std::endl;
          blk2.list(std::cout);

          vita::individual blk3(norm.replace(f,positions));
          std::cout << std::endl;
          blk3.list(std::cout);
          const boost::any val3(vita::interpreter(blk3).run());
          if (val3.empty())
            std::cout << "Incorrect output.";
          else
            std::cout << "Output: " << boost::any_cast<double>(val3);
          std::cout << std::endl << std::endl;

          if ( val.empty() != val3.empty() ||
               (!val.empty() && !val3.empty() &&
                boost::any_cast<double>(val) != 
                boost::any_cast<double>(val3)) )
          {
            std::cout << "ADF EVAL ERROR." << std::endl;
            return EXIT_FAILURE;
          }
        }
      }
      else
        std::cout << "Skipping block at line " << *i << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
