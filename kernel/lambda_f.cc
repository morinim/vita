/**
 *
 *  \file lambda_f.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "lambda_f.h"
#include "src_interpreter.h"

namespace vita
{
  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] d the training set.
  /// \param[in] x_slot number of slots for each class of the training set.
  ///
  /// Sets up the data structures needed by the 'dyn_slot' algorithm.
  ///
  dyn_slot_engine::dyn_slot_engine(const individual &ind, data &d,
                                   size_t x_slot)
    : slot_matrix(d.classes() * x_slot, uvect(d.classes())),
      slot_class(d.classes() * x_slot), dataset_size(0)
  {
    assert(ind.check());
    assert(d.check());
    assert(d.classes() > 1);
    assert(x_slot);

    const size_t n_slots(d.classes() * x_slot);
    assert(n_slots == slot_matrix.size());

    const data::dataset_t backup(d.dataset());
    d.dataset(data::training);

    // Here starts the slot-filling task.
    for (size_t i(0); i < n_slots; ++i)
    {
      assert(slot_matrix[i].size() == d.classes());

      for (size_t j(0); j < slot_matrix[i].size(); ++j)
        slot_matrix[i][j] = 0;
    }

    // In the first step this method evaluates the program to obtain an output
    // value for each training example. Based on the program output value a
    // a bidimentional array is built (slot_matrix_[slot][class]).
    for (auto &example : d)
    {
      ++dataset_size;

      const size_t where(slot(ind, example));

      ++slot_matrix[where][example.label()];
    }

    const size_t unknown(d.classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the
    // slot...
    for (size_t i(0); i < n_slots; ++i)
    {
      size_t best_class(0);

      for (size_t j(1); j < slot_matrix[i].size(); ++j)
        if (slot_matrix[i][j] >= slot_matrix[i][best_class])
          best_class = j;

      slot_class[i] = slot_matrix[i][best_class] ? best_class : unknown;
    }

    d.dataset(backup);
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] e input data for \a ind.
  /// \return the slot the example \a e falls into.
  ///
  size_t dyn_slot_engine::slot(const individual &ind,
                               const data::example &e) const
  {
    assert(ind.check());

    src_interpreter agent(ind);
    const any res(agent.run(e));

    const size_t ns(slot_matrix.size());
    const size_t last_slot(ns - 1);

    if (res.empty())
      return last_slot;

    const double val(interpreter::to_double(res));
    const size_t where(static_cast<size_t>(normalize_01(val) * ns));

    return (where >= ns) ? last_slot : where;
  }

  ///
  /// \param[in] x the numeric value that should be mapped in the [0,1]
  ///              interval.
  /// \return a double in the [0,1] range.
  ///
  /// This is a sigmoid function (it is a bounded real function, "S" shaped,
  /// with positive derivative everywhere).
  ///
  /// \see
  /// <http://en.wikipedia.org/wiki/Sigmoid_function>
  ///
  inline
  double dyn_slot_engine::normalize_01(double x)
  {
    // return (1.0 + x / (1 + std::fabs(x))) / 2.0;  // Algebraic function.

    return 0.5 + std::atan(x) / 3.1415926535;        // Arctangent.

    // return 0.5 + std::tanh(x);                    // Hyperbolic tangent.

    // return 1.0 / (1.0 + std::exp(-x));            // Logistic function.
  }

  ///
  /// \param[in] ind individual "to be transformed" into a lambda function.
  /// \param[in] d the training set.
  /// \param[in] x_slot number of slots for each class of the training set.
  ///
  dyn_slot_lambda_f::dyn_slot_lambda_f(const individual &ind, data &d,
                                       size_t x_slot)
    : lambda_f(ind), engine_(ind, d, x_slot), slot_name_(d.classes() * x_slot)
  {
    assert(ind.check());
    assert(d.check());
    assert(d.classes() > 1);
    assert(x_slot);

    assert(slot_name_.size() == engine_.slot_class.size());
    for (size_t i(0); i < slot_name_.size(); ++i)
      slot_name_[i] = d.class_name(engine_.slot_class[i]);
  }

  ///
  /// \param[in] instance data to be classified.
  /// \return the name of the class that includes \a instance.
  ///
  any dyn_slot_lambda_f::operator()(const data::example &instance) const
  {
    const size_t where(engine_.slot(ind_, instance));

    return any(slot_name_[where]);
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] d the training set.
  ///
  /// Sets up the data structures needed by the gaussian algorithm.
  ///
  gaussian_engine::gaussian_engine(const individual &ind, data &d)
    : gauss_dist(d.classes())
  {
    assert(ind.check());
    assert(d.classes() > 1);

    const data::dataset_t backup(d.dataset());
    d.dataset(data::training);

    src_interpreter agent(ind);

    // For a set of training data, we assume that the behaviour of a program
    // classifier is modelled using multiple Gaussian distributions, each of
    // which corresponds to a particular class. The distribution of a class is
    // determined by evaluating the program on the examples of the class in
    // the training set. This is done by taking the mean and standard deviation
    // of the program outputs for those training examples for that class.
    for (const auto &example : d)
    {
      const any res(agent.run(example));

      double val(res.empty() ? 0.0 : interpreter::to_double(res));
      const double cut(10000000);
      if (val > cut)
        val = cut;
      else if (val < -cut)
        val = -cut;

      gauss_dist[example.label()].add(val);
    }

    d.dataset(backup);
  }

  ///
  /// \param[in] ind program used for classification.
  /// \param[in] example input value whose class we are interested in.
  /// \param[out] prob
  /// \param[out] prob_sum
  /// \return the class of \a instance.
  ///
  size_t gaussian_engine::class_label(
    const individual &ind,
    const data::example &example,
    double *prob, double *prob_sum) const
  {
    const any res(src_interpreter(ind).run(example));
    const double x(res.empty() ? 0.0 : interpreter::to_double(res));

    double prob_(0.0), prob_sum_(0.0);
    size_t probable_class(0);

    for (size_t i(0); i < gauss_dist.size(); ++i)
    {
      const double distance(std::abs(x - gauss_dist[i].mean));
      const double variance(gauss_dist[i].variance);

      double p(0.0);
      if (variance == 0.0)     // These are borderline cases
        if (distance == 0.0)   // These are borderline cases
          p = 1.0;
        else
          p = 0.0;
      else                     // This is the standard case
        p = std::exp(-0.5 * distance * distance / variance);

      if (p > prob_)
      {
        prob_ = p;
        probable_class = i;
      }

      prob_sum_ += p;
    }

    if (prob)
      *prob = prob_;
    if (prob_sum)
      *prob_sum = prob_sum_;

    return probable_class;
  }

  ///
  /// \param[in] ind individual "to be transformed" into a lambda function.
  /// \param[in] d the training set.
  ///
  gaussian_lambda_f::gaussian_lambda_f(const individual &ind, data &d)
    : lambda_f(ind), engine_(ind, d), class_name_(d.classes())
  {
    assert(ind.check());
    assert(d.check());
    assert(d.classes() > 1);

    for (size_t i(0); i < d.classes(); ++i)
      class_name_[i] = d.class_name(i);
  }

  ///
  /// \param[in] instance data to be classified.
  /// \return the name of the class that includes \a instance.
  ///
  any gaussian_lambda_f::operator()(const data::example &instance) const
  {
    return any(class_name_[engine_.class_label(ind_, instance)]);
  }
}  // namespace vita
