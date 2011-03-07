/**
 *
 *  \file evaluator_proxy.h
 *
 *  \author Manlio Morini
 *  \date 2011/01/08
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(EVALUATOR_PROXY_H)
#define      EVALUATOR_PROXY_H

#include "vita.h"
#include "evaluator.h"
#include "ttable.h"

namespace vita
{

  ///
  /// Provide a surrogate for an \a evaluator to control access to it. The 
  /// reason for controlling access is to cache fitness scores of individuals.
  /// \c evaluator_proxy uses an ad-hoc internal transposition table 
  /// (\a ttable).     
  ///
  class evaluator_proxy : public evaluator 
  {
  public:
    evaluator_proxy(evaluator &, unsigned);

    void clear();

    fitness_t run(const individual &);

    boost::uint64_t probes() const;
    boost::uint64_t hits() const;

  private:
    /// Access to the real evaluator.
    evaluator &_eva;

    /// Transposition table (hash cache).
    ttable _cache;
  };

}  // namespace vita

#endif  // EVALUATOR_PROXY_H
