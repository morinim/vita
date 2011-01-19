/**
 *
 *  \file population.cc
 *
 *  \author Manlio Morini
 *  \date 2010/11/13
 *
 *  This file is part of VITA
 *
 */
  
#include <cstdlib>
#include <ctime>
#include <fstream>

#include "environment.h"
#include "population.h"
#include "random.h"

namespace vita
{

  ///
  /// \param[in] e base environment.
  ///
  population::population(environment &e) : _env(&e)
  {
    assert(e.check());

    build();
  }

  /**
   * build
   *
   * Creates a random population.
   */
  void
  population::build()
  {
    _pop.clear();

    for (unsigned i(0); i < _env->individuals; ++i)
      _pop.push_back(individual(*_env,true));

    assert(check());
  }

  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  bool
  population::check() const
  {
    for (unsigned i(0); i < size(); ++i)
      if (!_pop[i].check())
	return false;

    return _env;
  }

  /**
   * operator<<
   * \param[in,out] s
   * \param[in] pop
   * \return
   */
  std::ostream &
  operator<<(std::ostream &s, const population &pop)
  {
    for (population::const_iterator it(pop.begin()); it != pop.end(); ++it)
      s << *it << std::endl;
    
    return s;
  }

}  // namespace vita
