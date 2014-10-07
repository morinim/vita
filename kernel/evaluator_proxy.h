/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
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
  /// Provides a surrogate for an \a evaluator to control access to it. The
  /// reason for controlling access is to cache fitness scores of individuals.
  /// \c evaluator_proxy uses an ad-hoc internal hash table (\a ttable).
  ///
  template<class T>
  class evaluator_proxy : public evaluator<T>
  {
  public:
    evaluator_proxy(std::unique_ptr<evaluator<T>>, unsigned);

    virtual void clear(typename evaluator<T>::clear_flag) override;
    virtual void clear(const T &) override;

    virtual fitness_t operator()(const T &) override;
    virtual fitness_t fast(const T &) override;

    virtual double accuracy(const T &) const override;

    virtual std::string info() const override;

    virtual std::unique_ptr<lambda_f<T>> lambdify(const T &) const override;

    virtual unsigned seen(const T &) const override;

  private:
    // Access to the real evaluator.
    std::unique_ptr<evaluator<T>> eva_;

    // Transposition table (hash table cache).
    ttable cache_;
  };

#include "kernel/evaluator_proxy_inl.h"
}  // namespace vita

#endif  // include guard
