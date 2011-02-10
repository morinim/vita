/**
 *
 *  \file example4.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/31
 *
 *  This file is part of VITA
 *
 */

#include <cstdlib>
#include <iostream>

#include "environment.h"
#include "individual.h"
#include "primitive/sr_pri.h"
#include "ttable.h"

//
// Performs a speed test on the transposition table (insert-find cycle).
//
int main(int argc, char *argv[])
{
  vita::environment env;

  env.code_length = argc > 1 ? atoi(argv[1]) : 50;

  env.insert(new vita::sr::number(-200,200));
  env.insert(new vita::sr::add());
  env.insert(new vita::sr::sub());
  env.insert(new vita::sr::mul());
  env.insert(new vita::sr::ifl());
  env.insert(new vita::sr::ife());

  const unsigned n(argc > 2 ? atoi(argv[2]) : 5000);

  vita::ttable cache(argc > 3 ? atoi(argv[3]) : 16);

  const clock_t start(clock());
  for (unsigned i(0); i < n; ++i)
  {
    vita::individual i1(env,true);

    cache.insert(i1,i);

    vita::fitness_t f1;
    cache.find(i1,&f1);

    if (i % 1000 == 0)
      std::cout << i << '\r' << std::flush;
  }
  const clock_t end(clock());

  std::cout << double(n) * CLOCKS_PER_SEC / (end - start) << " store/read sec"
	    << std::endl;

  return EXIT_SUCCESS;
}
