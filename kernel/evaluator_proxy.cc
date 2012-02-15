/**
 *
 *  \file evaluator_proxy.cc
 *
 *  Copyright 2011 EOS di Manlio Morini.
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
 */

#include "kernel/evaluator_proxy.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \param[in] eva pointer that lets the proxy access the real evaluator.
  /// \param[in] ts 2^\a ts is the number of elements of the cache.
  ///
  evaluator_proxy::evaluator_proxy(evaluator *const eva, unsigned ts)
    : eva_(eva), cache_(ts)
  {
    assert(eva && ts);
  }

  ///
  /// \param[in] ind the individual whose fitness we want to know.
  /// \return the fitness and the accuracy of \a ind.
  ///
  eva_pair evaluator_proxy::operator()(const individual &ind)
  {
    eva_pair f;
    if (!cache_.find(ind, &f))
    {
      f = (*eva_)(ind);

      cache_.insert(ind, f);

#if !defined(NDEBUG)
      eva_pair f1;
      assert(cache_.find(ind, &f1));
      assert(f == f1);
#endif
    }

    /*
    #if !defined(NDEBUG)
    // Hash collision checking code can slow down the program very much.
    else
    {
      const fitness_t f1((*_eva)(ind));
      if (f != f1)
	std::cerr << "********* COLLISION *********" << std::endl;
    }
    #endif
    */

    return f;
  }

  ///
  /// Resets the evaluation caches.
  ///
  void evaluator_proxy::clear()
  {
    cache_.clear();
  }

  ///
  /// \return number of probes in the transposition table.
  ///
  boost::uint64_t evaluator_proxy::probes() const
  {
    return cache_.probes();
  }

  ///
  /// \return number of transposition table hits.
  ///
  boost::uint64_t evaluator_proxy::hits() const
  {
    return cache_.hits();
  }
}  // namespace vita
