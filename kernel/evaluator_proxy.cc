/**
 *
 *  \file evaluator_proxy.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "evaluator_proxy.h"
#include "individual.h"

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
  score_t evaluator_proxy::operator()(const individual &ind)
  {
    score_t s;
    if (!cache_.find(ind, &s))
    {
      s = (*eva_)(ind);

      cache_.insert(ind, s);

#if !defined(NDEBUG)
      score_t s1;
      assert(cache_.find(ind, &s1));
      assert(s == s1);
#endif
    }


    //#if !defined(NDEBUG)
    //else  // hash collision checking code can slow down the program very much
    //{
    //  const score_t s1((*_eva)(ind));
    //  if (s != s1)
    //  std::cerr << "********* COLLISION *********" << std::endl;
    //}
    //#endif

    return s;
  }

  ///
  /// Resets the evaluation cache.
  ///
  void evaluator_proxy::clear()
  {
    cache_.clear();
  }

  ///
  /// \param[in] ind an individual.
  ///
  /// Clears the cached informations for individual \a ind.
  ///
  void evaluator_proxy::clear(const individual &ind)
  {
    cache_.clear(ind);
  }

  ///
  /// \return number of probes in the transposition table.
  ///
  std::uintmax_t evaluator_proxy::probes() const
  {
    return cache_.probes();
  }

  ///
  /// \return number of transposition table hits.
  ///
  std::uintmax_t evaluator_proxy::hits() const
  {
    return cache_.hits();
  }
}  // namespace vita
