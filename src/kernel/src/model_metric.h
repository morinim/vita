/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_MODEL_METRIC_H)
#define      VITA_SRC_MODEL_METRIC_H

#include "kernel/lambda_f.h"
#include "kernel/vitafwd.h"
#include "kernel/src/data.h"

namespace vita
{
///
/// There are a lot of metrics related to a model (a lambda_f) and we don't
/// want fat classes.
/// The Visitor pattern is ideal to simplify the interface of lambda_f and
/// to keep possibility for future expansions (new metrics).
///
/// This works quite well since metrics can be implemented in terms of the
/// public interface of lambda_f.
///
template<class T>
class model_metric
{
public:
  virtual double operator()(const reg_lambda_f<T> *,
                            const src_data &) const = 0;
  virtual double operator()(const class_lambda_f<T> *,
                            const src_data &) const = 0;
};

///
/// Accuracy refers to the number of training examples that are correctly
/// valued/classified as a proportion of the total number of examples in
/// the training set. According to this design, the best accuracy is 1.0
/// (100%), meaning that all the training examples have been correctly
/// recognized.
///
/// \note
/// Accuracy and fitness aren't the same thing.
/// Accuracy can be used to measure fitness but it sometimes hasn't
/// enough "granularity"; also it isn't appropriated for classification
/// tasks with imbalanced learning data (where at least one class is
/// under/over represented relative to others).
///
template<class T>
class accuracy_metric : public model_metric<T>
{
public:
  virtual double operator()(const reg_lambda_f<T> *,
                            const src_data &) const override;

  virtual double operator()(const class_lambda_f<T> *,
                            const src_data &) const override;
};

#include "kernel/src/model_metric.tcc"
}  // namespace vita

#endif  // Include guard
