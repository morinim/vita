/**
 *
 *  \file evaluator_proxy.cc
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
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
    : _eva(eva), _cache(ts)
  {
    assert(eva->check() && ts);
  }

  ///
  /// \param[in] ind the individual whose fitness we want to know.
  /// \return the fitness of \a ind.
  ///
  fitness_t
  evaluator_proxy::run(const individual &ind)
  {
    fitness_t f;
    if (!_cache.find(ind,&f))
    {
      f = _eva->run(ind);
      _cache.insert(ind,f);
    }

    return f;    
  }

  ///
  /// Reset the transposition cache.
  ///
  void
  evaluator_proxy::clear()
  {
    _cache.clear();
  }

  ///
  /// \return number of probes in the transposition table.
  ///
  boost::uint64_t
  evaluator_proxy::probes() const
  {
    return _cache.probes();
  }

  ///
  /// \return number of transposition table hits.
  ///
  boost::uint64_t
  evaluator_proxy::hits() const
  {
    return _cache.hits();
  }

}  // namespace vita
