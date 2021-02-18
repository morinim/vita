/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVALUATOR_H)
#define      VITA_EVALUATOR_H

#include "kernel/fitness.h"
#include "kernel/gp/src/lambda_f.h"
#include "kernel/random.h"

namespace vita
{

class cached_evaluator
{
public:
  /// Clear possible cached values.
  /// \note The default implementation is empty.
  virtual void clear() {}
};

///
/// Calculates the fitness of an individual.
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
/// - bigger values represent better choices;
/// - (useful but not mandatory) optimal value is 0.
///
/// \warning
/// This class shouldn't be confused with the vita::interpreter class (that
/// calculates the output of an individual given an input vector).
///
template<class T>
class evaluator : public cached_evaluator
{
public:
  virtual ~evaluator() = default;

  /// \return the fitness of the individual
  virtual fitness_t operator()(const T &) = 0;

  // Serialization.
  virtual bool load(std::istream &);
  virtual bool save(std::ostream &) const;

  // The following methods have a default implementation (usually empty).
  virtual fitness_t fast(const T &);
  virtual std::unique_ptr<basic_lambda_f> lambdify(const T &) const;
};

enum class test_evaluator_type {distinct, fixed, random};

///
/// A fitness function used for debug purpose.
///
/// \tparam T  the type of individual used
/// \tparam ET the type of the test evaluator
///
/// Depending on `ET` the evaluator returns:
/// - a unique fitness (`test_evaluator_type::distinct`). Every individual has
///   his own (time invariant) fitness;
/// - a random (time invariant) fitness (`test_evaluator_type::random`);
/// - a constant fitness (`test_evaluator_type::fixed`). Same fitness for the
///   entire population.
///
template<class T>
class test_evaluator : public vita::evaluator<T>
{
public:
  explicit test_evaluator(test_evaluator_type = test_evaluator_type::random);

  fitness_t operator()(const T &) override;

private:
  std::vector<T>  buffer_;
  test_evaluator_type et_;
};
#include "kernel/evaluator.tcc"

}  // namespace vita

#endif  // include guard
