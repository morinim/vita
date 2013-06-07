/**
 *
 *  \file evaluator_proxy.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <boost/lexical_cast.hpp>

#include "evaluator_proxy.h"
#include "individual.h"
#include "lambda_f.h"

namespace vita
{
  ///
  /// \param[in] eva pointer that lets the proxy access the real evaluator.
  /// \param[in] ts 2^\a ts is the number of elements of the cache.
  ///
  evaluator_proxy::evaluator_proxy(std::unique_ptr<evaluator> eva, unsigned ts)
    : eva_(std::move(eva)), cache_(ts)
  {
    assert(eva);
    assert(ts > 6);
  }

  ///
  /// \param[in] ind the individual whose fitness we want to know.
  /// \return the fitness and the accuracy of \a ind.
  ///
  fitness_t evaluator_proxy::operator()(const individual &ind)
  {
    fitness_t f;
    if (cache_.find(ind, &f))
    {
      assert(cache_.hits());

#if defined(CLONE_SCALING)
      // Before evaluating an individual, we check if identical individuals
      // (clones) are already present in the population.
      // When the number of clones is grater than zero, the fitness assigned to
      // the individual is multiplied by  a clone-scaling factor.
      // For further details see "Evolving Assembly Programs: How Games Help
      // Microprocessor Validation" - F.Corno, E.Sanchez, G.Squillero.
      const double perc(double(cache_.seen(ind)) / cache_.hits());
      if (0.01 < perc && perc < 1.0)
        f -= (f * perc).abs() * 2.0;
#endif

      // Hash collision checking code can slow down the program very much.
#if !defined(NDEBUG)
      const fitness_t f1((*eva_)(ind));
      if (f[0] != f1[0])
        std::cerr << "********* COLLISION ********* [" << f
                  << " != " << f1 << "]" << std::endl;

      // In the above comparison we consider only the first component of the
      // fitness otherwise we can have false positives.
      // For example if the fitness is a 2D vector (where the first component
      // is the "score" on the training set and the second one is the effective
      // length of the individual), then the following two individuals:
      //
      // INDIVIDUAL A              INDIVIDUAL B
      // ------------------        ------------------
      // [000] FADD 001 002        [000] FADD 001 001
      // [001] X1                  [001] X1
      // [002] X1
      //
      // have the same signature, the same stored "score" but distinct
      // effective size and so distinct fitnesses.
#endif
    }
    else
    {
      f = (*eva_)(ind);

      cache_.insert(ind, f);

#if !defined(NDEBUG)
      fitness_t f1;
      assert(cache_.find(ind, &f1));
      assert(f == f1);
#endif
    }

    return f;
  }

  ///
  /// \param[in] what what should be cleared? (all, cache, stats)
  ///
  /// \brief Resets the evaluation cache / clear the statistics.
  ///
  void evaluator_proxy::clear(unsigned what)
  {
    switch (what)
    {
    case all:
    case cache:
      cache_.clear();
      break;

    case stats:
#if defined(CLONE_SCALING)
      cache_.reset_seen();
#endif
      break;
    }
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

  double evaluator_proxy::accuracy(const individual &ind) const
  {
    return eva_->accuracy(ind);
  }

  ///
  /// \return number of cache probes / hits.
  ///
  std::string evaluator_proxy::info() const
  {
    return
      "hits " + boost::lexical_cast<std::string>(cache_.hits()) +
      ", probes " + boost::lexical_cast<std::string>(cache_.probes()) +
      " (ratio " +
      boost::lexical_cast<std::string>(cache_.hits() * 100 / cache_.probes()) +
      "%)";
  }

  ///
  /// \param[in] ind an individual.
  /// \return a pointer to the executable version of \a ind.
  ///
  std::unique_ptr<lambda_f> evaluator_proxy::lambdify(
    const individual &ind) const
  {
    return eva_->lambdify(ind);
  }
}  // namespace vita
