/**
 *
 *  \file example8.cc
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
 *
 *  Building blocks run test.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "kernel/adf.h"
#include "kernel/distribution.h"
#include "kernel/environment.h"
#include "kernel/primitive/double_pri.h"

int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 5;
  const unsigned n(argc > 2 ? atoi(argv[2]) : 1);

  vita::symbol_ptr s1(new vita::sr::number(-200, 200));
  vita::symbol_ptr s2(new vita::sr::add());
  vita::symbol_ptr s3(new vita::sr::sub());
  vita::symbol_ptr s4(new vita::sr::mul());
  vita::symbol_ptr s5(new vita::sr::ifl());
  vita::symbol_ptr s6(new vita::sr::ife());
  vita::symbol_ptr s7(new vita::sr::abs());
  vita::symbol_ptr s8(new vita::sr::ln());
  env.insert(s1);
  env.insert(s2);
  env.insert(s3);
  env.insert(s4);
  env.insert(s5);
  env.insert(s6);
  env.insert(s7);
  env.insert(s8);

  for (unsigned k(0); k < n; ++k)
  {
    // We build, by repeated trials, an individual we an effective size greater
    // than 4.
    vita::individual base(env, true);
    unsigned base_es(base.eff_size());
    while (base_es < 5)
    {
      base = vita::individual(env, true);
      base_es = base.eff_size();
    }

    std::cout << std::string(40, '-') << std::endl << "BASE" << std::endl;
    base.list(std::cout);
    std::cout << std::endl;

    std::list<vita::locus_t> bl(base.blocks());
    for (auto i(bl.begin()); i != bl.end(); ++i)
    {
      vita::individual blk(base.get_block(*i));

      unsigned first_terminal;
      vita::individual opt(blk.optimize(&first_terminal));
      if (first_terminal)
      {
        std::cout << std::endl << "BLOCK at line " << *i << std::endl;
        blk.list(std::cout);
        const boost::any val((vita::interpreter(blk))());
        if (val.empty())
          std::cout << "Incorrect output.";
        else
          std::cout << "Output: " << boost::any_cast<double>(val);
        std::cout << std::endl;

        std::cout << std::endl << "OPTIMIZED" << std::endl;
        opt.list(std::cout);
        const boost::any val_n((vita::interpreter(opt))());

        if ( val.empty() != val_n.empty() ||
             (!val.empty() && !val_n.empty() &&
              boost::any_cast<double>(val) != boost::any_cast<double>(val_n)) )
        {
          std::cout << "OPTIMIZED BLOCK EVAL ERROR." << std::endl;
          return EXIT_FAILURE;
        }

        if (opt.eff_size() <= 20)
        {
          std::vector<vita::locus_t> positions;
          std::vector<vita::category_t> categories;
          vita::individual blk2(opt.generalize(2, &positions, &categories));
          vita::symbol_ptr f(new vita::adf(blk2, categories, 100));
          env.insert(f);
          std::cout << std::endl << f->display() << std::endl;
          blk2.list(std::cout);

          vita::individual blk3(opt.replace(f, positions));
          std::cout << std::endl;
          blk3.list(std::cout);
          const boost::any val3((vita::interpreter(blk3))());
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
