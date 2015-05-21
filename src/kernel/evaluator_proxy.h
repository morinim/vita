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

#if !defined(VITA_EVALUATOR_PROXY_H)
#define      VITA_EVALUATOR_PROXY_H

#include "kernel/evaluator.h"
#include "kernel/ttable.h"

namespace vita
{
///
/// \tparam T the type of individual used
///
/// Provides a surrogate for an evaluator to control access to it. The
/// reason for controlling access is to cache fitness scores of individuals.
/// evaluator_proxy uses an ad-hoc internal hash table (ttable).
///
template<class T>
class evaluator_proxy : public evaluator<T>
{
public:
  evaluator_proxy(std::unique_ptr<evaluator<T>>, unsigned);

  void clear(typename evaluator<T>::clear_flag);
  void clear(const T &);

  fitness_t operator()(const T &);
  fitness_t fast(const T &);

  std::string info() const;

  std::unique_ptr<lambda_f<T>> lambdify(const T &) const;

  unsigned seen(const T &) const;

private:
  // Access to the real evaluator.
  std::unique_ptr<evaluator<T>> eva_;

  // Transposition table (hash table cache).
  ttable cache_;
};

#include "kernel/evaluator_proxy.tcc"
}  // namespace vita

#endif  // include guard
