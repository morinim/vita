/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVALUATOR_H)
#define      VITA_EVALUATOR_H

#include "kernel/fitness.h"
#include "kernel/lambda_f.h"
#include "kernel/random.h"

namespace vita
{
///
/// \brief Calculates the fitness of an individual
///
/// \tparam T the type of individual used
///
/// `evaluator` class calculates the fitness of an individual (how good he is).
/// This is an abstract class because the fitness is domain dependent (symbolic
/// regression, data classification, automation...).
///
/// AKA **fitness function**.
///
/// \note
/// Our convention is to convert raw fitness to standardized fitness. The
/// requirements for standardized fitness are:
/// * bigger values represent better choices;
/// * (useful but not mandatory) optimal value is 0.
///
/// \warning
/// This class shouldn't be confused with the vita::interpreter class (that
/// calculates the output of an individual given an input vector).
///
template<class T>
class evaluator
{
public:
  enum clear_flag {cache = 1, stats = 2, all = cache | stats};

  /// \return the fitness of the individual.
  virtual fitness_t operator()(const T &) = 0;

  // The following methods have a default implementation (usually empty).
  virtual double accuracy(const T &) const;
  virtual void clear(clear_flag);
  virtual void clear(const T &);
  virtual fitness_t fast(const T &);
  virtual std::string info() const;
  virtual std::unique_ptr<lambda_f<T>> lambdify(const T &) const;
  virtual unsigned seen(const T &) const;
};

///
/// \brief A random fitness function used for debug purpose
///
/// \note
/// The output is population independent.
///
template<class T>
class random_evaluator : public evaluator<T>
{
public:
  virtual fitness_t operator()(const T &) override;
};

#include "kernel/evaluator.tcc"

}  // namespace vita

#endif  // Include guard
