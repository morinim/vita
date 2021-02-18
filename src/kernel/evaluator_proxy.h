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

#if !defined(VITA_EVALUATOR_PROXY_H)
#define      VITA_EVALUATOR_PROXY_H

#include "kernel/cache.h"
#include "kernel/evaluator.h"

namespace vita
{
///
/// Provides a surrogate for an evaluator to control access to it.
///
/// \tparam T the type of individual used
///
/// evaluator_proxy uses an ad-hoc internal hash table to cache fitness scores
/// of individuals.
///
template<class T, class E>
class evaluator_proxy : public evaluator<T>
{
public:
  evaluator_proxy(E, unsigned);

  // Serialization.
  bool load(std::istream &) override;
  bool save(std::ostream &) const override;

  void clear() override;

  fitness_t operator()(const T &) override;
  fitness_t fast(const T &) override;

  std::unique_ptr<basic_lambda_f> lambdify(const T &) const override;

private:
  // Access to the real evaluator.
  E eva_;

  // Hash table cache.
  cache cache_;
};

#include "kernel/evaluator_proxy.tcc"
}  // namespace vita

#endif  // include guard
